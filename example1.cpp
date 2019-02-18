#include "client.h"
#include "randomThings.h"

using namespace std::literals;

cerwy::task<void> bonkland() {
	cerwy::promise<int> promise;
	auto t = std::thread([&]() {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		promise.set_value(5);
	});
	int a = co_await promise.get_task();
	std::cout << a;
	t.detach();
}

//spam bot
int main() {
	//bonkland();
	//std::cin.get();
	//return 0;
	try {
		client c;
		std::vector<partial_message> msgs;
		c.on_guild_text_msg = [&](guild_text_message& msg, shard& s) {
			if (msg.content() == "watland") {
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
				s.change_nick(msg.guild(), i, std::string(msg.content()));
			}

			if (msg.author().id() != s.self_user().id())
				s.send_message(msg.channel(), std::to_string(msg.author().id().val));
			//s.add_reaction(wat,wat.guild().emojis().back());

		};
		c.on_guild_typing_start = [&](guild_member& member, text_channel& channel, shard& s) {
			s.send_message(channel, "rawr");
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
		c.on_guild_member_add = [&](guild_member& member, shard& s) {
			//s.send_message(member.guild().general_channel(),member.username()).get();
		};
		c.setToken(tokenType::BOT, getFileContents("token.txt"));
		c.run();
	} catch (...) {
		std::cout << ";-;" << std::endl;
	}
	std::cin.get();
}