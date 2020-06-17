#pragma once
#include <opus/opus.h>
#include <memory>
#include <vector>
#include <span>
#include "audio_source.h"
#include <iostream>
#include <fmt/format.h>

struct opus_encoder_deleter {
	void operator()(OpusEncoder* me) const noexcept {
		opus_encoder_destroy(me);
	}
};

struct opus_encoder {
	opus_encoder() = default;

	opus_encoder(OpusEncoder* t):
		m_me(t) {
		
		opus_encoder_ctl(m_me.get(),OPUS_SET_PACKET_LOSS_PERC(0.15));
	}

	explicit opus_encoder(opus_int32 sampling_rate, int channels, int application) {

		int error = 0;
		m_me = std::unique_ptr<OpusEncoder, opus_encoder_deleter>(opus_encoder_create(sampling_rate, channels, application, &error));
		if (error) {
			throw std::runtime_error("wat, can't init opus");
		}
	}
	
	std::vector<std::byte> encode(const audio_frame& frame,int max_size) {
		//fmt::print("{},{},{},{}\n",frame.frame_data.size(),frame.frame_size,frame.bit_rate,frame.channel_count);
		return encode(frame.frame_data, max_size, frame.frame_size);
	}

	void set_bit_rate(int bit_rate) {
		opus_encoder_ctl(m_me.get(), OPUS_SET_BITRATE(bit_rate));		
	}

private:
	std::unique_ptr<OpusEncoder, opus_encoder_deleter> m_me = nullptr;

	// ReSharper disable CppMemberFunctionMayBeConst
	std::vector<std::byte> encode(std::span<int16_t> input_audio, int max_size, int frame_size) {
		// ReSharper restore CppMemberFunctionMayBeConst
		
		//set_bit_rate(64*1024);
		std::vector<std::byte> return_data(max_size);		
		const auto len = opus_encode(m_me.get(), input_audio.data(), frame_size, (unsigned char*)return_data.data(), max_size);
		
		if (len < 0) {
			throw std::runtime_error("");
		}
		else {
			return_data.resize(len);
			return return_data;
		}
	}
	
};



