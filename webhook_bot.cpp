#include <fstream>
#include "include/client.h"


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

int main() {
	client c;
	c.on_guild_text_msg = [](const guild_text_message& msg, shard& s)->cerwy::task<void> {
		if (msg.content() == "rawrly") {
			auto hook = co_await s.create_webhook(msg.channel(),"bester_hook");
			s.send_with_webhook(hook, "rawrworld");
			
		}
	};

	c.set_token(getFileContents("token.txt"), token_type::BOT);

	c.run();
	return 0;
}
