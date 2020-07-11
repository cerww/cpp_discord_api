#include <fstream>
#include "include/client.h"
#include "include/webhook_client.h"


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

//https://discordapp.com/api/webhooks/731038085143461959/Ox6E4tx7HniojkhZjIXgOPyvXL8dqcrHLM_GcNfOCnj5cDgN17iAOogiZNlST5CaYvAf

int main() {

	webhook_client c2(snowflake(731038085143461959ull),"Ox6E4tx7HniojkhZjIXgOPyvXL8dqcrHLM_GcNfOCnj5cDgN17iAOogiZNlST5CaYvAf");
	c2.send("aaa");
	std::thread t([&]() {
		c2.ioc().run();
	});

	
	client c;
	c.on_guild_text_msg = [](const guild_text_message& msg, shard& s)-> cerwy::task<void> {
		if (msg.content() == "rawrly") {
			auto hook = co_await s.create_webhook(msg.channel(), "bester_hook");
			co_await s.send_with_webhook(hook, "rawrworld");
			auto wh_client = s.make_webhook_client(hook);				
			co_await wh_client.send("potatoland");
			co_await wh_client.send("potatoland2");
			
			co_await s.modify_webhook(hook,modify_webhook().name("charizard"));
			
		}
	};

	c.set_token(getFileContents("token.txt"), token_type::BOT);

	c.run();
	return 0;
}
