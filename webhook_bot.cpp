#include <fstream>
#include "include/client.h"
#include "include/webhook_client.h"
#include "allowed_mentions.h"


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

inline void aihsjgdasiodgasidjasd() {
	nlohmann::json v = allowed_mentions().add_roles(snowflake(111), snowflake(111)).add_roles(snowflake(111)).all_users();
	v = allowed_mentions().all_users().all_roles();
	std::cout << v.dump(4) << std::endl;
	int aasdjhasd = 0;
}

int main() {
	aihsjgdasiodgasidjasd();

	
	webhook_client c2(snowflake(731786996493844591ull),"k8ElkHsnKrD83NkWiRniQhRHbR_JsXgiF038MecLLCem2bxcYaI2UuH74cOn6QwEWX9-");
	c2.send("aaa");
	std::thread t([&]() {
		c2.run();
	});	

	
	
	client c;
	c.on_guild_text_msg = [](const guild_text_message& msg, shard& s)-> cerwy::task<void> {
		if (msg.content() == "rawrly") {
			const auto hook = co_await s.create_webhook(msg.channel(), "bester_hook");
			co_await s.send_with_webhook(hook, "rawrworld");
			auto wh_client = s.make_webhook_client(hook);				
			co_await wh_client.send("potatoland");
			co_await wh_client.send("potatoland2");
			
			co_await s.modify_webhook(hook,modify_webhook_settings().name("charizard"));
			
		}
	};

	c.set_token(getFileContents("token.txt"), token_type::BOT);

	c.run();
	return 0;
}
