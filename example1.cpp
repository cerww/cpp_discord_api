#include "client.h"
#include <fstream>
#include "async_mutex.h"
#include "modify_guild_settings.h"
#include "mp3_audio_source.h"


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


cerwy::task<void> do_audio_thingy(cerwy::task<voice_connection> vc_task) {
	voice_connection channel = co_await vc_task;
	//co_await channel.send(audio_from_mp3("C:/Users/cerw/Downloads/a2.mp3"));
	co_await channel.send(mp3_audio_source(from_file{ "C:/Users/cerw/Downloads/a2.mp3" }));
	
}

cerwy::task<void> thingy(const Guild& g,shard& s) {
	while(true) {
		boost::asio::steady_timer timer(s.strand());
		timer.expires_after(20s);
		co_await timer.async_wait(use_task);
		auto& role = g.roles()[snowflake(725880768659980330)];
		s.send_message(s.text_channels().at(snowflake(504562059279728640)),role.to_mentionable_string() + " flag starts soon");
	}
}

//spam bot
int main() {
	//thingy_to_debugy();


	//std::cin.get();
	//try {
	client c;
	
	boost::asio::io_context::strand* shard_of_guild = nullptr;


	cerwy::task<voice_connection> connashk;
	std::vector<partial_message> msgs;

	c.on_guild_ready = [&](const Guild& g, shard& s) {
		if(g.id() == snowflake(199218706029608961)) {
			shard_of_guild = &s.strand();
			//thingy(g, s);
		}

		
	};

	//c.on_guild_ready = &thingy;
	
	c.on_guild_text_msg = [&](guild_text_message msg, shard& s) {
		if (msg.content() == "rawrmander") {
			//int i = 0;
			/*
			for (auto&& a : s.voice_channels()) {
				std::cout << a.first.val << ' ' << a.second.guild_id().val << std::endl;;
			}
			*/
			for (auto&& a : msg.guild().voice_channel_ids()) {
				std::cout << a.val << ' ' << std::endl;
			}
			do_audio_thingy(s.connect_voice(msg.guild().voice_channels()[0]));
		}
		else if (msg.content() == "watland") {
			//s.delete_message(msgs.back()).get();
			//msgs.pop_back();
		}
		else if (msg.content() == "make new channel") {
			auto ch = s.create_text_channel(msg.guild(), "blargylandy").get();
		}
		else if (msg.content() == "rolesy") {
			std::string stuff =
					msg.author().roles() |
					ranges::views::transform(&guild_role::name) |
					ranges::views::join(" "sv) |
					ranges::to<std::string>();

			s.send_message(msg.channel(), stuff);
		}
		else if (msg.content() == "invite") {
			s.create_channel_invite(msg.channel()).wait();
		}
		else if (msg.content() == "namey") {
			s.send_message(msg.channel(), std::string(msg.author().nick()));
		}else if(msg.content() == ";-;worldlandasdasdasd") {
			const optional_ref<const guild_role> role = msg.guild().role_by_name("Prophets of Milktea-ism");
			if (role.has_value()) {
				s.send_message(msg.channel(), role->to_mentionable_string());
			}
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
		s.send_message(member.guild().system_channel(), "rawr");
		//s.modify_guild(member.guild(), guild_settings::default_message_notifications{1});
		//auto t = modify_guild_settings().name("aaa");
	};

	c.setToken(getFileContents("token.txt"), token_type::BOT);
	c.run();
	//} catch (...) {
	//std::cout << ";-;" << std::endl;
	//}
	std::cin.get();
	return 0;
}
