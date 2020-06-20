#include "mp3_audio_source.h"
#define MINIMP3_IMPLEMENTATION
#include <minimp3/minimp3.h>
#include <minimp3/minimp3_ex.h>
#include <fmt/format.h>
#include <fstream>
#include <array>


mp3_audio_source::mp3_audio_source(::from_file f) {

	//mp3dec_init(&m_mp3d);
	from_file(std::move(f.file_name));
}
/*
static void mp3dec_skip_id3_aaa(const uint8_t** pbuf, size_t* pbuf_size)
{
    char* buf = (char*)(*pbuf);
    size_t buf_size = *pbuf_size;
    if (buf_size > 10 && !strncmp(buf, "ID3", 3))
    {
        size_t id3v2size = (((buf[6] & 0x7f) << 21) | ((buf[7] & 0x7f) << 14) |
            ((buf[8] & 0x7f) << 7) | (buf[9] & 0x7f)) + 10;
        buf += id3v2size;
        buf_size -= id3v2size;
    }

    if (buf_size > 128 && !strncmp(buf + buf_size - 128, "TAG", 3))
    {
        buf_size -= 128;
        if (buf_size > 227 && !strncmp(buf + buf_size - 227, "TAG+", 4))
            buf_size -= 227;
    }
	
    * pbuf = (const uint8_t*)buf;
    *pbuf_size = buf_size;
}
*/

//skips ID3, or sets it back to std::ios::beg, returns number of chars read
int skip_id3(std::ifstream& file) {
    std::array<char, 3> id3{};

    file.read(id3.data(), 3);
    
	if(std::string_view(id3.data(), 3) == ("ID3")) {
        std::array<char, 7> meta_meta_data{};
        file.read(meta_meta_data.data(), 7);
		
        const int id3v2size = (((id3[6] & 0x7f) << 21) | ((id3[7] & 0x7f) << 14) |
            ((id3[8] & 0x7f) << 7) | (id3[9] & 0x7f));

        std::vector<char> id3_rest(id3v2size);

        file.read(id3_rest.data(), id3v2size);
        return id3v2size + 10;
		
	}else if(std::string_view(id3.data(), 3) == ("TAG")) {
        char c;
        file.read(&c, 1);
		if(c=='+') {
			//id3v1 plus, 227 chars total, 4 read, 223 chars left
            std::array<char, 223> full_id3v1{};
            file.read(full_id3v1.data(), full_id3v1.size());
            return 227;
		}else {
			//id3v1, 128 chars total, 4 read , 124 chars left
            std::array<char, 124> full_id3v1{};
            file.read(full_id3v1.data(), full_id3v1.size());
            return 128;
		}
	}else {
        file.seekg(0, std::ios::beg);
        return 0;
	}
}

void mp3_audio_source::from_file(std::string file_name) {    
    std::ifstream file(file_name, std::ios::binary);
    file.seekg(0, std::ios::end);
    int filesize = (int)file.tellg();
    file.seekg(0, std::ios::beg);
    filesize -= (int)file.tellg();//always 0? idk i copy-pasted this part

    if(filesize>=4) {
        filesize -= skip_id3(file);
    }
	
	
    m_file_data.resize(filesize);
	//file.s
    file.read((char*)m_file_data.data(), filesize);    
    file.close();
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
