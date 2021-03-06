#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include "../common/ref_count_ptr.h"
#include "snowflake.h"
#include "../common/web_socket_session_impl.h"
#include "../common/task_completion_handler.h"
#include "guild.h"
#include "../common/opus_encoder.h"
#include "../common/resume_on_strand.h"



struct discord_voice_connection_impl :
		ref_counted {

	explicit discord_voice_connection_impl(web_socket_session sock, boost::asio::io_context& ioc);

	discord_voice_connection_impl(const discord_voice_connection_impl&) = delete;
	discord_voice_connection_impl(discord_voice_connection_impl&&) = delete;

	discord_voice_connection_impl& operator=(discord_voice_connection_impl&&) = delete;

	discord_voice_connection_impl& operator=(const discord_voice_connection_impl&) = delete;

	~discord_voice_connection_impl() = default;

	void start() {
		send_identify();
		socket.start_reads();
	}

	decltype(auto) context() {
		return socket.socket().get_executor();
	}

	cerwy::eager_task<void> control_speaking(int is_speaking);

	cerwy::eager_task<void> send_silent_frames();

	void close();

	template<typename T>
	cerwy::eager_task<void> send_voice(const T& data) {
		using namespace std::literals;
		using std::chrono::duration_cast;

		co_await control_speaking(1);
		is_playing = true;

		uint16_t sqeuence_number = 0;
		const auto ssrc_big_end = htonl(ssrc);
		static constexpr auto time_frame = 20ms;

		auto last_time_sent_packet = std::chrono::steady_clock::now();

		for (audio_frame frame : data.frames(time_frame)) {
			if (is_canceled.exchange(false)) {
				is_playing = false;
				co_await control_speaking(0);
				co_await resume_on_strand(*strand);
				cancel_promise.set_value();
				co_return;
			}

			if (frame.channel_count != 2 || frame.sampling_rate != 48000) {
				frame = resample_fn(frame, 2, 48000);
			}
			co_await send_frame(frame, sqeuence_number++, ssrc_big_end);

			last_time_sent_packet += time_frame;
			const auto next_time_to_send_packet = last_time_sent_packet;

			co_await wait(duration_cast<std::chrono::milliseconds>(next_time_to_send_packet - std::chrono::steady_clock::now()));
		}

		is_playing = false;

		co_await control_speaking(0);
		co_await resume_on_strand{*strand};

		if (is_canceled.exchange(false)) {
			cancel_promise.set_value();
			co_return;
		}
	};

	template<typename T>
	cerwy::eager_task<void> send_async(T&& async_source) {
		using namespace std::literals;
		using std::chrono::duration_cast;

		co_await control_speaking(1);
		is_playing = true;

		uint16_t sqeuence_number = 0;
		const auto ssrc_big_end = htonl(ssrc);
		static constexpr auto time_frame = 20ms;

		auto last_time_sent_packet = std::chrono::steady_clock::now();

		//TODO change with for co_await when that is a thing
		auto frames = async_source.frames(time_frame);
		while (true) {
			std::optional<audio_frame> frame_opt = co_await frames.next();
			if (!frame_opt) {
				break;
			}
			audio_frame& frame = *frame_opt;
			if (is_canceled.exchange(false)) {
				is_playing = false;
				co_await control_speaking(0);
				co_await resume_on_strand{*strand};
				cancel_promise.set_value();
				co_return;
			}

			if (frame.channel_count != 2 || frame.sampling_rate != 48000) {
				frame = resample_fn(frame, 2, 48000);
			}
			co_await send_frame(frame, sqeuence_number++, ssrc_big_end);

			last_time_sent_packet += time_frame;
			const auto next_time_to_send_packet = last_time_sent_packet;

			co_await wait(duration_cast<std::chrono::milliseconds>(next_time_to_send_packet - std::chrono::steady_clock::now()));
		}

		is_playing = false;

		co_await control_speaking(0);
		co_await resume_on_strand{*strand};

		if (is_canceled.exchange(false)) {
			cancel_promise.set_value();
			co_return;
		}
	}

	cerwy::eager_task<void> cancel_current_data() {
		if (!is_playing || is_canceled) {
			return cerwy::make_ready_void_task();
		}
		cancel_promise = cerwy::promise<void>();
		is_canceled = true;
		return cancel_promise.get_task();
	}

	snowflake channel_id;
	snowflake my_id;
	snowflake guild_id;

	std::string token;
	std::string web_socket_endpoint;
	std::string session_id;

	boost::asio::io_context::strand* strand = nullptr;

	web_socket_session socket;

	int heartbeat_interval = 0;
	uint32_t ssrc = 0;

	ref_count_ptr<const Guild> guild;

	cerwy::promise<void>* waiter = nullptr;//only used in setup

	int delay = 0;
	std::atomic<bool> is_alive = true;

	boost::asio::ip::udp::socket voice_socket;

	std::atomic<bool> is_playing = false;
	std::atomic<bool> is_canceled = false;
	cerwy::promise<void> cancel_promise;
	std::function<audio_frame(const audio_frame&, int, int)> resample_fn = resample_easy;
private:
	std::string m_ip;
	int m_port = 0;
	int m_hb_number = 1;
	std::vector<std::string> m_modes;
	opus_encoder m_opus_encoder = opus_encoder(48000, 2, OPUS_APPLICATION_AUDIO);
	uint32_t m_timestamp = 0;
	boost::asio::ip::udp::endpoint m_my_endpoint;

	std::vector<std::byte> m_secret_key;

	//header is nonce
	std::vector<std::byte> encrypt_xsalsa20_poly1305(std::array<std::byte, 12> header, std::span<const std::byte> audio_data);

	cerwy::eager_task<void> send_heartbeat();

	void on_msg_recv(nlohmann::json data, int opcode);
	void on_hello(nlohmann::json d);
	void send_identify();
	void on_ready(nlohmann::json data);
	void send_op1_select_protocol() const;
	void on_session_discription(nlohmann::json data);

	cerwy::eager_task<void> connect_udp();
	cerwy::eager_task<void> do_ip_discovery();

	//TODO remove these once i get modules
	[[nodiscard]] cerwy::eager_task<boost::system::error_code> send_voice_data_udp(std::span<const std::byte>);

	[[nodiscard]] cerwy::eager_task<boost::system::error_code> wait(std::chrono::milliseconds);

	cerwy::eager_task<void> send_frame(const audio_frame& frame, uint16_t sqeuence_number, uint32_t ssrc_big_end) {
		std::array<std::byte, 12> header{{}};

		(uint8_t&)header[0] = 0x80;
		(uint8_t&)header[1] = 0x78;

		(uint16_t&)header[2] = htons(sqeuence_number);

		(uint32_t&)header[4] = htonl(m_timestamp);
		(uint32_t&)header[8] = ssrc_big_end;

		std::array<std::byte, 1000> opus_data{};//save 1 memory allocation!!!!!!
		const int len = m_opus_encoder.encode_into_buffer(frame, opus_data.data(), (int)opus_data.size());

		const auto encrypted_voice_data = encrypt_xsalsa20_poly1305(header, std::span<std::byte>(opus_data.data(), len));
		co_await send_voice_data_udp(encrypted_voice_data);//don't acually need to check if data was sent?
		m_timestamp += frame.frame_size;
	}
};

template<typename T>
concept async_audio_source = requires(T s)
{
	{ s.frames(std::chrono::milliseconds(1)) };
	{ s.frames().next().await_resume() }-> std::convertible_to<bool>;
	{ *s.frames().next().await_resume() }-> std::same_as<audio_frame>;
	{ s.frames().next().await_ready() }-> std::convertible_to<bool>;
};

template<typename T>
concept audio_source = requires(T s)
{
	{ s.frames(std::chrono::milliseconds(1)) }->std::ranges::range;
	{ *s.frames(std::chrono::milliseconds(1)).begin() }-> std::convertible_to<audio_frame>;
};

struct voice_connection {
	voice_connection() = default;

	voice_connection(const voice_connection&) = delete;
	voice_connection& operator=(const voice_connection&) = delete;

	voice_connection(voice_connection&&) = default;
	voice_connection& operator=(voice_connection&&) = default;

	~voice_connection() {
		disconnect();
	};

private:
	explicit voice_connection(ref_count_ptr<discord_voice_connection_impl> connection):
		m_connection(std::move(connection)) {}

public:
	void disconnect() {
		if (m_connection) {
			//use .async_* lambda pyramids to close?
			m_connection->close();
			m_connection = nullptr;
		}
	}

	template<typename T>//audio_source
	cerwy::eager_task<void> send(const T& data) {
		return m_connection->send_voice(data);
	};

	template<typename T>//audio_source
	cerwy::eager_task<void> send_async(T&& data) {
		return m_connection->send_async(std::forward<T>(data));
	};

	//todo: rename
	cerwy::eager_task<void> cancel_current_data() {
		return m_connection->cancel_current_data();
	}

	template<typename F>
	void set_resample_fn(F&& f) {
		m_connection->resample_fn = std::forward<F>(f);
	}

	const Guild& guild() const noexcept {
		return *m_connection->guild;
	}

	const voice_channel& channel() const {
		const auto& guild = this->guild();
		auto it = ranges::find(guild.voice_states(), m_connection->my_id, &voice_state::user_id);
		if (it == guild.voice_states().end()) {
			throw std::runtime_error("already disconnected");
		}
		const voice_state& v = *it;

		return guild.voice_channels()[v.channel_id()];
	}

	boost::asio::io_context::strand& strand() {
		return *m_connection->strand;
	}

	const boost::asio::io_context::strand& strand() const {
		return *m_connection->strand;
	}

	bool is_connected() const noexcept {
		return m_connection != nullptr;
	}

private:
	ref_count_ptr<discord_voice_connection_impl> m_connection = nullptr;
	friend cerwy::eager_task<std::pair<voice_connection, ref_count_ptr<discord_voice_connection_impl>>> voice_connect_impl(internal_shard& me, const voice_channel& ch, std::string endpoint, std::string token, std::string session_id);
};
