#include "mp3_audio_source.h"
#define MINIMP3_IMPLEMENTATION
#include <minimp3/minimp3.h>
#include <minimp3/minimp3_ex.h>
#include <fmt/format.h>
#include <fstream>


mp3_audio_source::mp3_audio_source(::from_file f) {

	mp3dec_init(&m_mp3d);
	from_file(std::move(f.file_name));
}

void mp3_audio_source::from_file(std::string file_name) {
    
    /*mp3dec_file_info_t info;
    if (mp3dec_load(&m_mp3d, file_name.c_str(), &info, nullptr, nullptr))  {
        throw std::runtime_error("failed to open file");
    }
	

	//info.samples == size of buffer	

    m_sampling_rate = info.hz;
    m_channel_count = info.channels;
	*/
    std::ifstream file(file_name, std::ios::binary);
    file.seekg(0, std::ios::end);
    int filesize = (int)file.tellg();
    file.seekg(0, std::ios::beg);
    filesize -= (int)file.tellg();
    //fileContents.resize(filesize);
    //file.read(fileContents.data(), filesize);
    m_file_data.resize(filesize);
    file.read((char*)m_file_data.data(), filesize);
    //mp3dec
    file.close();
	/*
    mp3dec_map_info_t map_info;
    mp3dec_open_file(file_name.c_str(),&map_info);
	
    mp3dec_close_file(&map_info);
	*/
    //mp3dec_skip_id3();
}

audio_data audio_from_mp3(std::string file_name) {
    mp3dec_t mp3_dec = {};
    mp3dec_init(&mp3_dec);
    mp3dec_file_info_t info;
    if (mp3dec_load(&mp3_dec, file_name.c_str(), &info, nullptr, nullptr)) {
        auto thingy_to_make_sure_buffer_is_freed = std::unique_ptr<int16_t>(info.buffer);
        throw std::runtime_error("failed to open file");
    }

    //info.samples == size of buffer	
    
    // m_sampling_rate = info.hz;
    // m_channel_count = info.channels;
    // m_bit_rate = info.avg_bitrate_kbps;
    
	auto thingy_to_make_sure_buffer_is_freed = std::unique_ptr<int16_t>(info.buffer);
    auto data = std::vector<int16_t>(info.buffer,info.buffer + info.samples);
    fmt::print("{},{},{}\n",info.hz,info.channels,info.avg_bitrate_kbps);
    return audio_data(std::move(data), info.hz, info.channels, info.avg_bitrate_kbps*1024);
}
