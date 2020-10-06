#include "../command_parser/command_context.h"
#include "../include/client.h"
#include <fstream>
#include "../common/ytdl_audio_source.h"
#include <boost/process.hpp>
#include <stack>

using namespace std::literals;
using namespace fmt::literals;

std::string getFileContents(const std::string& filePath, decltype(std::ios::in) mode = std::ios::in) {
	std::string fileContents;
	std::ifstream file(filePath, mode);
	file.seekg(0, std::ios::end);
	const int filesize = (int)file.tellg();
	fileContents.resize(filesize);
	file.seekg(0, std::ios::beg);	
	file.read((char*)fileContents.data(), filesize);
	file.close();
	return fileContents;
}


int main() {
	client c;
	c.set_token(getFileContents("token.txt"));
	shiny::command_context command_thingy;
	command_thingy.prefix = "!!!";

	command_thingy["aaa"] += shiny::make_command<guild_role>([](const guild_role&,guild_text_message m, shard& s) {
		s.send_message(m.channel(), "watworld").execute_and_ignore();
	});

	command_thingy["bonk"] += shiny::make_command<guild_member, guild_role>(
		[](const guild_member& member, const guild_role& role,guild_text_message m,shard& s) {
			std::cout << role.id().val << std::endl;
			s.send_message(m.channel(), "wat {}, {}"_format(member, role)).execute_and_ignore();
		});

	command_thingy["join"] += shiny::make_command([](guild_text_message m,shard& s)->cerwy::task<void> {
		auto channel_opt = m.guild().voice_channel_for(m.author());
		if (!channel_opt) {
			co_await s.send_message(m.channel(), "not connected");
		} else {
			auto connection = co_await s.connect_voice(channel_opt.value());
			co_await connection.send_async(ytdl_source("https://www.youtube.com/watch?v=5lfLO3ZWfAg", s.strand().context()));
		}
	});

	command_thingy["say"] += shiny::make_command<std::string>([](std::string str, guild_text_message m,shard& s) {
		s.send_message(m.channel(), std::move(str)).execute_and_ignore();
	});
	
	c.on_guild_text_msg = [&](guild_text_message msg,shard& s) {
		if (msg.author_id() == s.self_user().id()) {
			return;
		}
		auto& channel = msg.channel();
		command_thingy.do_command(std::move(msg), s);
		s.send_message(channel, "aaa").execute_and_ignore();
	};
	c.run();
}
