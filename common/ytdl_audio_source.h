#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include "audio_source.h"
#include "task.h"
#include "task_completion_handler.h"
#include <optional>


struct ytdl_async_audio {
	explicit ytdl_async_audio(std::string url, boost::asio::io_context& ioc) :
		m_url(std::move(url)),
		m_ioc(ioc) { }

	struct async_thingy {
		explicit async_thingy(std::string url, boost::asio::io_context& ioc) :
			ioc_(ioc),
			ytdl_pipe(std::make_unique<boost::process::async_pipe>(ioc)),
			ffmpeg_pipe(std::make_unique<boost::process::async_pipe>(ioc)),
			ytdl_child(std::make_unique<boost::process::child>(
				fmt::format("youtube-dl -f bestaudio  \"{}\" -o - --buffer-size 16384", url), boost::process::std_out > *ytdl_pipe,
				boost::process::std_err > stderr, ioc)),
			ffmpeg_child(std::make_unique<boost::process::child>(
				"ffmpeg -re -i - -f s16le -ac 2 -ar 48000 -acodec pcm_s16le -",
				boost::process::std_out > *ffmpeg_pipe, boost::process::std_in < *ytdl_pipe, boost::process::std_err > stderr, ioc)) { }

		cerwy::task<std::optional<audio_frame>> next() {
			//samples needed = 1920 = frame_time * 48000/1000 * 2 /sizeof(int16_t)


			std::vector<int16_t> buffer(1920);

			auto [ec, n] = co_await boost::asio::async_read(
				*ffmpeg_pipe,
				boost::asio::buffer((char*)buffer.data(), 1920 * 2),
				use_task_return_tuple2);

			if (ec) {
				co_return std::nullopt;
			}

			//if(ec == boost::asio::error::basic_errors::broken_pipe);

			//const auto aaa = std::string_view((const char*)samples.data(), 1920);

			audio_frame frame;
			frame.optional_data_storage = std::move(buffer);
			frame.channel_count = 2;
			frame.sampling_rate = 48000;
			frame.frame_size = 960;
			frame.frame_data = frame.optional_data_storage;

			co_return std::optional<audio_frame>(std::move(frame));
		}

		boost::asio::io_context& ioc_;
		std::unique_ptr<boost::process::async_pipe> ytdl_pipe;
		std::unique_ptr<boost::process::async_pipe> ffmpeg_pipe;
		std::unique_ptr<boost::process::child> ytdl_child;
		std::unique_ptr<boost::process::child> ffmpeg_child;
		//std::vector<int16_t> excess;

		std::array<std::byte, 4096> buffer2 = {};
		int number_of_bytes = 0;
	};

	async_thingy frames(std::chrono::milliseconds) {
		return async_thingy(m_url, m_ioc);
	}

private:
	std::string m_url;
	boost::asio::io_context& m_ioc;
};
