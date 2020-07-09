#pragma once
#include "audio_source.h"
#include <minimp3/minimp3.h>
#include "iterator_facade.h"
#include <variant>
#include <cassert>

struct from_file {
	std::string file_name;
};


struct mp3_audio_source {
	mp3_audio_source() = default;


	explicit mp3_audio_source(from_file f);

	mp3_audio_source(const mp3_audio_source&) = delete;
	mp3_audio_source& operator=(const mp3_audio_source&) = delete;

	mp3_audio_source(mp3_audio_source&&) = default;
	mp3_audio_source& operator=(mp3_audio_source&&) = default;

	~mp3_audio_source() = default;
	
	struct frames_range {
		static constexpr int max_samples_size_for_stack_storage = 960 * 2;//20ms for 48k hz 2 channels
		static constexpr int array_size = MINIMP3_MAX_SAMPLES_PER_FRAME + max_samples_size_for_stack_storage;
		
		explicit frames_range(const std::span<const std::byte> data, const std::chrono::milliseconds f_milliseconds) :
			file_data(data),
			frame_length(f_milliseconds) {

			mp3dec_init(&mp3dec);
		}

		struct cursor {
			cursor() = default;

			cursor(std::span<const std::byte> data, mp3dec_t* dec, std::chrono::milliseconds f_len):
				m_file_data(data),
				m_mp3dec(dec),
				m_frame_length(f_len),
				m_pcm_variant(std::in_place_type<std::array<int16_t, array_size>>)/*for init*/ {

				auto& pcm_for_init = std::get<std::array<int16_t,array_size>>(m_pcm_variant);

				//read 1 frame to get sampleing_rate and channels
				while (m_samples_in_thingy == 0 && !m_file_data.empty()) {
					mp3dec_frame_info_t frame_info;
					const int samples = mp3dec_decode_frame(m_mp3dec, (uint8_t*)m_file_data.data(),(int) m_file_data.size(), pcm_for_init.data(), &frame_info);

					m_file_data = m_file_data.subspan(frame_info.frame_bytes);
					
					m_channels = frame_info.channels;
					m_sampling_rate = frame_info.hz;
					m_current_frame_bitrate = frame_info.bitrate_kbps * 1024;
					
					m_samples_in_thingy += samples * m_channels;
				}

				if (samples_needed() > max_samples_size_for_stack_storage) {
					//switch to vector if more space is needed
					std::vector<int16_t> temp(((int)MINIMP3_MAX_SAMPLES_PER_FRAME + samples_needed()));
					std::copy(pcm_for_init.begin(), pcm_for_init.end(), temp.begin());
					m_pcm_variant = std::move(temp);
				}

				decode_enough_samples();
			}

			audio_frame read() {

				return audio_frame{
					.frame_data = std::span<int16_t>(pcm().data(), samples_needed()),
					.sampling_rate = m_sampling_rate,
					.channel_count = m_channels,
					.bit_rate = m_current_frame_bitrate,
					.frame_size = samples_needed() / m_channels
				};
			}

			void next() {
				remove_1_frame();

				decode_enough_samples();
			}
			
			//bool operator==(const cursor& other) = default;

			bool done()const {
				return m_file_data.empty();				
			}

		private:
			std::span<const std::byte> m_file_data = {};
			mp3dec_t* m_mp3dec = nullptr;
			std::chrono::milliseconds m_frame_length = std::chrono::milliseconds(0);
			
			std::variant<
				std::array<int16_t, array_size>,
				std::vector<int16_t>
			> m_pcm_variant;
			
			int m_samples_in_thingy = 0;
			int m_sampling_rate = 0;
			int m_channels = 1;
			int m_current_frame_bitrate = 0;

			
			int samples_needed() const {
				return m_sampling_rate * (int)m_frame_length.count() * m_channels / 1000;
			}


			//by default, use std::array, unless samples_needed is too large, then use heap
			std::span<int16_t> pcm() {
				if (samples_needed() > MINIMP3_MAX_SAMPLES_PER_FRAME) {
					return std::get<std::vector<int16_t>>(m_pcm_variant);
				} else {
					return std::get<std::array<int16_t, array_size>>(m_pcm_variant);
				}
			}

			void decode_enough_samples() {
				while (m_samples_in_thingy < samples_needed() && !m_file_data.empty()) {
					mp3dec_frame_info_t frame_info;

					const int samples = mp3dec_decode_frame(
						m_mp3dec, 
						(uint8_t*)m_file_data.data(), 
						(int)m_file_data.size(), 
						pcm().data() + m_samples_in_thingy, 
						&frame_info
					);

					m_file_data = m_file_data.subspan(frame_info.frame_bytes);
					m_samples_in_thingy += samples * m_channels;
					m_current_frame_bitrate = frame_info.bitrate_kbps * 1024;
				}
			}

			void remove_1_frame() {
				assert(m_samples_in_thingy >= samples_needed());
				std::shift_left(pcm().begin(), pcm().begin() + m_samples_in_thingy, samples_needed());
				m_samples_in_thingy -= samples_needed();
			}

		};

		using iterator = iterator_facade<cursor>;

		iterator begin() noexcept{
			return iterator(file_data, &mp3dec, frame_length);
		}

		static ranges::default_sentinel_t end() noexcept {
			return {};
		}
		
	private:
		std::span<const std::byte> file_data;
		std::chrono::milliseconds frame_length;
		mp3dec_t mp3dec = {};
	};

	auto frames(std::chrono::milliseconds frame_length)const {
		return frames_range(m_file_data,frame_length);
	}

private:
	std::vector<std::byte> m_file_data = {};

	void from_file(std::string file_name);

};

audio_data audio_from_mp3(std::string file_name);
