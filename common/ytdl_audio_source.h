#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include "audio_source.h"
#include "eager_task.h"
#include "task_completion_handler.h"
#include <optional>


struct ytdl_source {
	explicit ytdl_source(std::string url, boost::asio::io_context& ioc) :
		m_url(std::move(url)),
		m_ioc(ioc) { }

	explicit ytdl_source(std::string url, boost::asio::io_context::strand& ioc) :
		m_url(std::move(url)),
		m_ioc(ioc.context()) { }

	struct async_thingy {
		explicit async_thingy(std::string_view url, boost::asio::io_context& ioc,std::chrono::milliseconds t_time_frame) :
			ioc_(ioc),
			ytdl_pipe(std::make_unique<boost::process::async_pipe>(ioc)),
			ffmpeg_pipe(std::make_unique<boost::process::async_pipe>(ioc)),
			ytdl_child(
				fmt::format("youtube-dl -f bestaudio  \"{}\" -o - --buffer-size 8192", url), 
				boost::process::std_out > *ytdl_pipe,
				//boost::process::std_err > stderr, 
				boost::process::std_err.null(),
				ioc),
			ffmpeg_child(
				"ffmpeg -re -i - -f s16le -ac 2 -ar 48000 -acodec pcm_s16le -",
				boost::process::std_out > *ffmpeg_pipe, boost::process::std_in < *ytdl_pipe, 
				//boost::process::std_err > stderr,
				boost::process::std_err.null(),
				ioc),
			time_frame(t_time_frame){ }

		cerwy::eager_task<std::optional<audio_frame>> next() const{
			//samples needed = 1920 = frame_time * 48000n * 20/1000 * 2 /sizeof(int16_t) 
			constexpr int channel_count = 2;
			constexpr int sampling_rate = 48000;
			const int samples = sampling_rate * (int)time_frame.count() / 1000;

			std::vector<int16_t> buffer(samples * channel_count);

			auto [ec, n] = co_await boost::asio::async_read(
				*ffmpeg_pipe,
				boost::asio::buffer(buffer),
				use_task_return_tuple2);

			if (ec) {
				co_return std::nullopt;
			}

			//if(ec == boost::asio::error::basic_errors::broken_pipe);

			//const auto aaa = std::string_view((const char*)samples.data(), 1920);

			audio_frame frame;
			frame.optional_data_storage = std::move(buffer);
			frame.channel_count = channel_count;
			frame.sampling_rate = sampling_rate;
			frame.frame_size = samples;
			frame.frame_data = frame.optional_data_storage;

			co_return std::optional(std::move(frame));
		}

		boost::asio::io_context& ioc_;
		std::unique_ptr<boost::process::async_pipe> ytdl_pipe;
		std::unique_ptr<boost::process::async_pipe> ffmpeg_pipe;
		boost::process::child ytdl_child;
		boost::process::child ffmpeg_child;
		std::chrono::milliseconds time_frame;
	};

	async_thingy frames(std::chrono::milliseconds a) {
		return async_thingy(m_url, m_ioc,a);
	}

private:
	std::string m_url;
	boost::asio::io_context& m_ioc;
};

struct ytdl_search_source {
	explicit ytdl_search_source(std::string query, boost::asio::io_context& ioc) :
		m_search_query(std::move(query)),
		m_ioc(ioc) { }

	explicit ytdl_search_source(std::string query, boost::asio::io_context::strand& ioc) :
		m_search_query(std::move(query)),
		m_ioc(ioc.context()) { }

	struct async_thingy {
		explicit async_thingy(std::string_view query, boost::asio::io_context& ioc, std::chrono::milliseconds t_time_frame) :
			ioc_(ioc),
			ytdl_pipe(std::make_unique<boost::process::async_pipe>(ioc)),
			ffmpeg_pipe(std::make_unique<boost::process::async_pipe>(ioc)),
			ytdl_child(
				fmt::format("youtube-dl -f bestaudio  ytsearch1:\"{}\" -o - --buffer-size 8192 --no-playlist", query), boost::process::std_out > * ytdl_pipe,
				boost::process::std_err.null(),
				ioc),
			ffmpeg_child(
				"ffmpeg -re -i - -f s16le -ac 2 -ar 48000 -acodec pcm_s16le -",
				boost::process::std_out > * ffmpeg_pipe, boost::process::std_in < *ytdl_pipe, 
				boost::process::std_err.null(),
				ioc),
			time_frame(t_time_frame) { }

		cerwy::eager_task<std::optional<audio_frame>> next()const {
			//samples needed = 1920 = frame_time * 48000n * 20/1000 * 2 /sizeof(int16_t) 
			constexpr int channel_count = 2;
			constexpr int sampling_rate = 48000;
			const int samples = sampling_rate * (int)time_frame.count() / 1000;

			std::vector<int16_t> buffer(samples * channel_count);

			auto [ec, n] = co_await boost::asio::async_read(
				*ffmpeg_pipe,
				boost::asio::buffer(buffer),
				use_task_return_tuple2);

			if (ec) {
				co_return std::nullopt;
			}

			//if(ec == boost::asio::error::basic_errors::broken_pipe);

			//const auto aaa = std::string_view((const char*)samples.data(), 1920);

			audio_frame frame;
			frame.optional_data_storage = std::move(buffer);
			frame.channel_count = channel_count;
			frame.sampling_rate = sampling_rate;
			frame.frame_size = samples;
			frame.frame_data = frame.optional_data_storage;

			co_return std::optional(std::move(frame));
		}

		boost::asio::io_context& ioc_;
		std::unique_ptr<boost::process::async_pipe> ytdl_pipe;
		std::unique_ptr<boost::process::async_pipe> ffmpeg_pipe;
		boost::process::child ytdl_child;
		boost::process::child ffmpeg_child;
		std::chrono::milliseconds time_frame;
	};

	//gib async_generator
	async_thingy frames(std::chrono::milliseconds a) const{
		return async_thingy(m_search_query, m_ioc, a);
	}

private:
	std::string m_search_query;
	boost::asio::io_context& m_ioc;
};

