#include "client.h"
#include <fstream>
#include "async_mutex.h"
using namespace std::literals;

std::string getFileContents(const std::string& filePath, decltype(std::ios::in) mode = std::ios::in) {
	std::string fileContents;
	std::ifstream file(filePath, mode);
	file.seekg(0, std::ios::end);
	int filesize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	filesize -= (int)file.tellg();
	fileContents.resize(filesize);
	file.read(fileContents.data(), filesize);
	file.close();
	return fileContents;
}

//spam bot
int main() {
	//thingy_to_debugy();


	//std::cin.get();
	//try {
		client c;
		
		cerwy::task<voice_connection> connashk;
		std::vector<partial_message> msgs;		
		c.on_guild_text_msg = [&](const guild_text_message& msg, shard& s) {
			if (msg.content() == "rawrmander") {
				//int i = 0;
				for(auto&& a:s.voice_channels()) {
					std::cout << a.first.val << ' ' << a.second.guild_id().val << std::endl;;
				}
				for(auto&& a:msg.guild().voice_channel_ids()) {
					std::cout << a.val << ' ' << std::endl;
				}
				connashk = s.connect_voice(msg.guild().voice_channels()[0]);
			}else if (msg.content() == "watland") {
				//s.delete_message(msgs.back()).get();
				//msgs.pop_back();
			} else if (msg.content() == "make new channel") {
				s.create_text_channel(msg.guild(), "blargylandy").wait();
			} else if (msg.content() == "rolesy") {
				std::string stuff =
					msg.author().roles() |
					ranges::view::transform(&guild_role::name) |
					ranges::view::join(" "sv) |
					ranges::to_<std::string>();

				s.send_message(msg.channel(), stuff);
			}else if (msg.content() == "invite") {
				s.create_channel_invite(msg.channel()).wait();
			}else if(msg.content() == "namey") {
				s.send_message(msg.channel(), std::string(msg.author().nick()));
			}
			//s.change_nick(wat.author(), wat.content());

			for (const auto& i : msg.mentions()) {
				s.change_nick(i, std::string(msg.content()));
			}

			if (msg.author().id() != s.self_user().id())
				s.send_message(msg.channel(), std::to_string(msg.author().id().val));
			//s.add_reaction(wat,wat.guild().emojis().back());
			
		};
		c.on_guild_typing_start = [&](const guild_member& member, const text_channel& channel, shard& s) {
			s.send_message(channel, "rawr");
			std::cout << "rawr" << std::endl;
			/*
			msgs.push_back(s.send_message(channel,member.username()+ " has started typing").get());
			s.send_message(channel, member.username() + " has started typing");
			s.send_message(channel, member.username() + " has started typing");
			s.send_message(channel, member.username() + " has started typing");
			s.send_message(channel, member.username() + " has started typing");
			s.send_message(channel, member.username() + " has started typing");
			s.send_message(channel, member.username() + " has started typing");
			s.send_message(channel, member.username() + " has started typing").wait();
			member.guild().roles();
			*/
		};
		c.on_guild_member_add = [&](const guild_member& member, shard& s) {
			//s.send_message(member.guild().general_channel(),member.username()).get();
		};
		c.setToken(token_type::BOT, getFileContents("token.txt"));		
		c.run();
	//} catch (...) {
		//std::cout << ";-;" << std::endl;
	//}
	std::cin.get();
}



