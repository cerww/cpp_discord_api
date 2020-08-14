#include "../command_parser/command_context.h"
#include "../include/client.h"
#include <fstream>

using namespace std::literals;

std::string getFileContents(const std::string& filePath, decltype(std::ios::in) mode = std::ios::in) {
	std::string fileContents;
	std::ifstream file(filePath, mode);
	file.seekg(0, std::ios::end);
	int filesize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	filesize -= (int)file.tellg();
	fileContents.resize(filesize);
	file.read((char*)fileContents.data(), filesize);
	file.close();
	return fileContents;
}

using namespace fmt::literals;

int main() {
	client c;
	c.set_token(getFileContents("token.txt"));
	shiny::command_context command_context;
	command_context.prefix = "!!!";

	command_context["aaa"] += shiny::make_command<guild_role>([](const guild_role&,guild_text_message m, shard& s) {
		s.send_message(m.channel(), "watworld");
	});

	command_context["bonk"] += shiny::make_command<guild_member, guild_role>(
		[](const guild_member& member, const guild_role& role,guild_text_message m,shard& s) {
			std::cout << role.id().val << std::endl;
			s.send_message(m.channel(), "wat {}, {}"_format(member.to_mentionable_string(), role.to_mentionable_string()));
		});


	c.on_guild_text_msg = [&](guild_text_message msg,shard& s) {
		if (msg.author_id() == s.self_user().id()) {
			return;
		}
		auto& channel = msg.channel();
		command_context.do_command(std::move(msg), s);
		s.send_message(channel, "aaa");
	};
	c.run();
}
