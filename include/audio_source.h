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

audio_frame transform_data(const audio_frame& frame,int new_channel_count, int new_sampling_rate) {
	const auto frame_length = std::chrono::milliseconds(frame.frame_size / new_channel_count / new_sampling_rate * 1000);
	const size_t needed_size = new_channel_count * new_sampling_rate * (int)frame_length.count() / 1000;
	
	audio_frame return_val;
	return_val.optional_data_storage.resize(needed_size);
	const double size_ratio = (double)needed_size / (double)frame.frame_data.size();
	
	//hmmm
	return_val.optional_data_storage[0] = frame.frame_data[0];
	for (int i = 1; i < frame.frame_data.size(); ++i) {
		const int new_idx = i * size_ratio;
		return_val.optional_data_storage[new_idx] = frame.frame_data[i];
		
		const int prev_entry_new_index = (i - 1) * size_ratio;
		
	}
	
	
	return return_val;
}

struct audio_source_base {
	audio_source_base() = default;

	
	explicit audio_source_base(int sampling_rate, int channels) :
		m_sampling_rate(sampling_rate),
		m_channel_count(channels)
	
	{}
	
	int sampleing_rate() const noexcept { return m_sampling_rate; }

	int channels() const noexcept { return m_channel_count; }

protected:
	int m_sampling_rate = 0;
	int m_channel_count = 0;
};

struct audio_data:audio_source_base {

	audio_data() = default;

	explicit audio_data(std::vector<int16_t> data,int sampling_rate, int channels, int bit_rate):
		audio_source_base(sampling_rate,channels),
		m_audio_data(std::move(data)),
		m_bit_rate(bit_rate)
	{}

	

	auto frames(std::chrono::milliseconds frame_length)const  {
		//assert frame_length == allowed frames
		const int sample_size = ((sizeof(int16_t)) * m_channel_count)/sizeof(int16_t);
		const int samples_per_frame = int(m_sampling_rate * frame_length.count()) / 1000;
		const int frame_size = samples_per_frame * sample_size;

		//4,960,1920
		fmt::print("{},{},{}",sample_size,samples_per_frame,frame_size);
		
		return
		m_audio_data
		| ranges::views::take(m_audio_data.size()/frame_size * frame_size)//this ensures the size is a mul;tiple of frame_size,so it doesn't read beyond the buffer, 
		| ranges::views::chunk(frame_size)
		| ranges::views::transform([=/*vars go out of scope, only copying ints*/](auto a) {
			return audio_frame{
				.frame_data = std::span<int16_t>((int16_t*)&a[0],frame_size),
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

