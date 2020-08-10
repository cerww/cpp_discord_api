#include "../no_cache/client.h"
#include <fstream>

using namespace cacheless;

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
	c.set_token(getFileContents("token.txt"));

	c.on_guild_text_msg = [](events::guild_message_create event,shard& s) {
		if(event.msg.content == "wat") {
			s.send_message(event.msg.channel_id, "a+b=c");
		}
	};
	
	c.run();	
}

