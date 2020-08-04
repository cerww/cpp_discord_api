#pragma once
#include <chrono>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/transform.hpp>
#include <span>
#include <fmt/format.h>

namespace opus_params {//copied from discord.py
	constexpr int sampling_rate = 48000;
	constexpr int channels = 2;
	constexpr int frame_length = 20;
	constexpr int sample_size = 4;//# (bit_rate / 8) * CHANNELS   (bit_rate == 16)
	constexpr int samples_per_frame = int(sampling_rate * frame_length) / 1000;

	constexpr int frame_size = samples_per_frame * sample_size;
}


struct audio_frame {
	std::span<int16_t> frame_data = {};
	int sampling_rate = 0;
	int channel_count = 0;
	int bit_rate = 0;
	int frame_size = 0;

	std::vector<int16_t> optional_data_storage = {};
};

inline audio_frame resample_meh(const audio_frame& frame, int new_channel_count, int new_sampling_rate) {

	if (new_channel_count == frame.channel_count && new_sampling_rate == frame.sampling_rate) {
		const bool frame_is_using_storage = frame.frame_data.data() == frame.optional_data_storage.data();
		if (!frame_is_using_storage) {
			return frame;
		} else {
			auto ret = frame;
			ret.frame_data = ret.optional_data_storage;
			return ret;
		}
	}

	const auto frame_length = std::chrono::milliseconds(frame.frame_size * 1000 / frame.sampling_rate);
	const size_t needed_size = new_channel_count * new_sampling_rate * (int)frame_length.count() / 1000;

	audio_frame return_val;
	return_val.channel_count = new_channel_count;
	return_val.sampling_rate = new_sampling_rate;
	return_val.frame_size = new_sampling_rate / 1000 * (int)frame_length.count();
	return_val.optional_data_storage.resize(needed_size);
	return_val.frame_data = return_val.optional_data_storage;

	auto& new_frame = return_val.optional_data_storage;

	const double size_ratio = (double)needed_size / (double)frame.frame_data.size();
	const double sampling_rate_ratio = (double)new_sampling_rate / (double)frame.sampling_rate;


	int last_idx_used = -1;

	for (int i = 0; i < return_val.frame_size; ++i) {
		const size_t idx = (size_t)i * new_channel_count;
		const int idx_in_unsampled = int((double)i / sampling_rate_ratio) * frame.channel_count;

		if (new_channel_count == 1 && frame.channel_count == 1) {
			new_frame[idx] = frame.frame_data[idx_in_unsampled];
		} else if (new_channel_count == 2 && frame.channel_count == 2) {
			(uint32_t&)new_frame[idx] = (uint32_t&)frame.frame_data[idx_in_unsampled];
		} else if (new_channel_count == 2 && frame.channel_count == 1) {
			new_frame[idx] = new_frame[idx + 1] = frame.frame_data[idx_in_unsampled];
		} else {
			for (int j = 0; j < new_channel_count; ++j) {
				new_frame[size_t(idx + j)] = frame.frame_data[(size_t)((size_t)idx_in_unsampled + (size_t)std::min(j, frame.channel_count - 1))];
			}
		}

		if (idx_in_unsampled != last_idx_used + 1) {
			const int num_samples_in_between = idx_in_unsampled - last_idx_used;
			for (int j = last_idx_used; j < idx_in_unsampled; j += new_channel_count) {
				const float t = float(j - last_idx_used) / (float)num_samples_in_between;

				for (int k = 0; k < new_channel_count; ++k) {
					new_frame[(size_t)j + (size_t)k] =
							(int16_t)std::lerp(
								(float)new_frame[(size_t)last_idx_used + (size_t)k],
								(float)new_frame[(size_t)idx_in_unsampled + (size_t)k],
								t);
				}
			}
		}
		if (idx_in_unsampled != last_idx_used) {

			last_idx_used = idx_in_unsampled;
		}
	}
	return return_val;
}

inline audio_frame resample_easy(const audio_frame& frame, const int new_channel_count, const int new_sampling_rate) {
	if (new_channel_count == frame.channel_count && new_sampling_rate == frame.sampling_rate) {
		const bool frame_is_using_storage = frame.frame_data.data() == frame.optional_data_storage.data();
		if (!frame_is_using_storage) {
			return frame;
		} else {
			auto ret = frame;
			ret.frame_data = ret.optional_data_storage;
			return ret;
		}
	}

	const auto frame_length = std::chrono::milliseconds(frame.frame_size * 1000 / frame.sampling_rate);
	const size_t needed_size = new_channel_count * new_sampling_rate * (int)frame_length.count() / 1000;

	audio_frame return_val;
	return_val.channel_count = new_channel_count;
	return_val.sampling_rate = new_sampling_rate;
	return_val.frame_size = new_sampling_rate * (int)frame_length.count() / 1000 ;
	return_val.optional_data_storage.resize(needed_size);
	return_val.frame_data = return_val.optional_data_storage;

	auto& new_frame = return_val.optional_data_storage;

	const double sampling_rate_ratio = (double)new_sampling_rate / (double)frame.sampling_rate;

	for (int i = 0; i < return_val.frame_size; ++i) {
		const size_t idx = (size_t)i * new_channel_count;
		const int idx_in_unsampled = int((double)i / sampling_rate_ratio) * frame.channel_count;

		if (new_channel_count == 1 && frame.channel_count == 1) {
			new_frame[idx] = frame.frame_data[idx_in_unsampled];
		} else if (new_channel_count == 2 && frame.channel_count == 2) {
			(uint32_t&)new_frame[idx] = (uint32_t&)frame.frame_data[idx_in_unsampled];
		} else if (new_channel_count == 2 && frame.channel_count == 1) {
			new_frame[idx] = new_frame[idx + 1] = frame.frame_data[idx_in_unsampled];
		} else {
			for (int j = 0; j < new_channel_count; ++j) {
				new_frame[size_t(idx + j)] = frame.frame_data[(size_t)((size_t)idx_in_unsampled + (size_t)std::min(j, frame.channel_count - 1))];
			}
		}
	}
	return return_val;
}


struct audio_source_base {
	audio_source_base() = default;


	explicit audio_source_base(int sampling_rate, int channels) :
		m_sampling_rate(sampling_rate),
		m_channel_count(channels) {}

	int sampleing_rate() const noexcept { return m_sampling_rate; }

	int channels() const noexcept { return m_channel_count; }

protected:
	int m_sampling_rate = 0;
	int m_channel_count = 0;
};

struct audio_data :audio_source_base {

	audio_data() = default;

	explicit audio_data(std::vector<int16_t> data, int sampling_rate, int channels, int bit_rate):
		audio_source_base(sampling_rate, channels),
		m_audio_data(std::move(data)),
		m_bit_rate(bit_rate) {}


	auto frames(std::chrono::milliseconds frame_length) const {
		//assert frame_length == allowed frame lengths 
		
		const int sample_size = ((sizeof(int16_t)) * m_channel_count) / sizeof(int16_t);
		const int samples_per_frame = int(m_sampling_rate * frame_length.count()) / 1000;
		const int frame_size = samples_per_frame * sample_size;

		return
				m_audio_data
				| ranges::views::take(m_audio_data.size() / frame_size * frame_size)//this ensures the size is a multiple of frame_size,so it doesn't read beyond the buffer, 
				| ranges::views::chunk(frame_size)
				| ranges::views::transform([=/*vars go out of scope, only copying ints*/](auto a) {
					return audio_frame{
						.frame_data = std::span<int16_t>((int16_t*)&a[0], frame_size),
						.sampling_rate = m_sampling_rate,
						.channel_count = m_channel_count,
						.bit_rate = m_bit_rate,
						.frame_size = samples_per_frame
					};
				});
	}

private:
	std::vector<int16_t> m_audio_data;
	int m_bit_rate = 0;
};
