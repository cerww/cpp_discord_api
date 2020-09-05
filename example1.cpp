#include "include/client.h"
#include <fstream>
#include "include/modify_guild_settings.h"
#include "common/mp3_audio_source.h"
#include "common/more_bad_vector.h"


using namespace std::literals;

std::string getFileContents(const std::string& filePath, decltype(std::ios::in) mode = std::ios::in) {
	std::string fileContents;
	std::ifstream file(filePath, mode);
	file.seekg(0, std::ios::end);
	const int filesize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	fileContents.resize(filesize);
	file.read((char*)fileContents.data(), filesize);
	file.close();
	return fileContents;
}

cerwy::task<void> do_audio_thingy(cerwy::task<voice_connection> vc_task) {
	voice_connection channel = co_await vc_task;
	//co_await channel.send(audio_from_mp3("C:/Users/cerw/Downloads/a2.mp3"));
	co_await channel.send(mp3_audio_source(from_file{"C:/Users/cerw/Downloads/a.mp3"}));

}

cerwy::task<void> thingy(const Guild& g, shard& s) {
	while (true) {
		boost::asio::steady_timer timer(s.strand());
		timer.expires_after(20s);
		co_await timer.async_wait(use_task);
		auto& role = g.roles()[snowflake(725880768659980330)];
		//s.send_message(s.text_channels().at(snowflake(504562059279728640)), role.to_mentionable_string() + " flag starts soon");
	}
}

//test for crashing
void test_transform_audio_thingy() {
	audio_frame framy;
	framy.optional_data_storage.resize(44100 * 20 * 2 / 1000);

	framy.channel_count = 2;
	framy.sampling_rate = 44100;
	framy.frame_size = framy.sampling_rate * 20 / 1000;
	framy.frame_data = framy.optional_data_storage;
	auto transformed_framy = resample_meh(framy, 2, 48000);
	int breakpoint_here = 0;
}

void aujsdhasdasd() {
	std::vector<int> aaa = { 12,3,4,5,67,7,8 };
	more_bad_vector<int> y = aaa | ranges::to<more_bad_vector<int>>();
	assert(y[1] == aaa[1]);
	auto x = std::move(y);
	assert(y.size() == 0);
	more_bad_vector<int> asdasd;
	assert(asdasd.size() == 0);
	assert(asdasd.empty());

	asdasd = more_bad_vector<int>(aaa.begin(), aaa.end());
	//assert(ranges::equal(aaa, asdasd));
	
	std::span<int> wat = x;
	std::vector<int> c = wat | ranges::to<std::vector>();
	std::cout << (aaa == c) << std::endl;;
	
}

//spam bot
int main() {
	//test_transform_audio_thingy();
	//thingy_to_debugy();

	//std::cin.get();
	//try {
	//aujsdhasdasd();
	//std::cin.get();
	client c;

	boost::asio::io_context::strand* shard_of_guild = nullptr;

	cerwy::task<voice_connection> connashk;
	std::vector<partial_message> msgs;

	c.on_guild_ready = [&](const Guild& g, shard& s) {
		if (g.id() == snowflake(199218706029608961)) {
			shard_of_guild = &s.strand();
			//thingy(g, s);
		}
	};

	//c.on_guild_ready = &thingy;

	c.on_guild_text_msg = [&](guild_text_message msg, shard& s)->cerwy::task<void> {
		if (msg.content() == "rawrmander") {			
			do_audio_thingy(s.connect_voice(msg.guild().voice_channels_list()[0]));
		} else if (msg.content() == "watland") {
			//s.delete_message(msgs.back()).get();
			//msgs.pop_back();
		} else if (msg.content() == "make new channel") {
			auto ch = co_await s.create_text_channel(msg.guild(), "blargylandy");
		} else if (msg.content() == "rolesy") {
			//std::cout << "rolesy" << std::endl;
			std::string stuff =
					msg.author().roles() |
					ranges::views::transform(&guild_role::name) |
					ranges::views::join(" "sv) |
					ranges::to<std::string>();

			s.reply(msg, stuff);
		} else if (msg.content() == "invite") {
			co_await s.create_channel_invite(msg.channel()).async_wait();
		} else if (msg.content() == "namey") {
			s.send_message(msg.channel(), std::string(msg.author().nick()));
		} else if (msg.content() == ";-;worldlandasdasdasd") {
			const auto role = msg.guild().role_by_name("Prophets of Milktea-ism");
			if (role.has_value()) {
				s.send_message(msg.channel(), role->to_mentionable_string(),disable_mentions);
			}
		} else if (msg.content() == "potatoland_world") {
			s.send_message(msg.channel(), "rawr",
						   embed()
						   .set_author(embed_author().set_name("cerwtato"))
						   .set_footer(embed_footer().set_text("azumarill"))
						   .set_color(0x99a520)
						   .set_description("wat")
						   .add_fields(
							   embed_field().set_name("abc").set_value("a+b=c"),
							   embed_field()
							   .set_is_inline(true).set_name("better field").set_value("awesome"),
							   embed_field().set_is_inline(true).set_name("more awesome").set_value("best field")
						   )
			);
		} else if (msg.content() == "hamtaroland_worldy") {
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
			s.send_message(msg.channel(), "charmander");
		} else if (msg.content() == "at everyone") {
			s.send_message(msg.channel(), "@everyone", allowed_mentions());
		} else if (msg.content() == "at petery") {
			s.send_message(msg.channel(), "<@188547243911938048>", disable_mentions);
		} else if (msg.content() == "hello kitty") {
			const auto new_msg = co_await s.send_message(msg.channel(), "charmanderworld");
			co_await s.add_reaction(new_msg, msg.guild().emojis().front());
			co_return;

		} else if (msg.content() == "formosaland") {
			auto logs = co_await s.get_audit_log(msg.guild());
			std::string str;
			// TODO create better interface

			for (const auto& entry : logs.entries()) {
				str += fmt::format("{},{},{},{}\n", entry.id().val, (int)entry.action_type(), entry.user_id().val, entry.target_id().value_or(snowflake(0)).val);
				for (const audit_log_change& change : entry.changes()
					 | ranges::views::filter([&](const auto& a) {
						 return a.keys_that_are_int.contains(a.key())
								 || a.keys_that_are_bool.contains(a.key())
								 || a.keys_that_are_string.contains(a.key())
								 || a.keys_that_are_snowflake.contains(a.key());//only these keys cuz i need to format them

					 })
				) {

					std::visit([&](const auto& old_value, const auto& new_value) {
						using old_type = std::decay_t<decltype(old_value)>;
						using new_type = std::decay_t<decltype(new_value)>;

						if constexpr (!std::is_same_v<old_type, new_type>) {
							return;
						} else {

							using type = old_type;

							if constexpr (
								std::is_same_v<type, std::optional<changed_role>> ||
								std::is_same_v<type, std::optional<std::vector<permission_overwrite>>>) {
									
								str += fmt::format("\t{:>30}, old:{:>20}, new:{:>20} \n", change.key(), "wat", "wat");
									
							} else {
								if (old_value.has_value() && new_value.has_value()) {
									str += fmt::format("\t{:>30}, old:{:>20}, new:{:>20} \n", change.key(), old_value.value(), new_value.value());
								} else if (old_value.has_value()) {
									str += fmt::format("\t{:>30}, old:{:>20}, new:{:>20} \n", change.key(), old_value.value(), "nope");
								} else if (new_value.has_value()) {
									str += fmt::format("\t{:>30}, old:{:>20}, new:{:>20} \n", change.key(), "nope", new_value.value());
								} else {
									str += fmt::format("\t{:>30}, old:{:>20}, new:{:>20} \n", change.key(), "nope", "nope");
								}
							}
						}
					}, change.old_value(), change.new_value());
				}
			}
			std::fstream file = std::fstream("abcd.txt", std::ios::out);

			file << str;
			file.close();

		}else if(msg.content() == "charizard_world") {
			auto m1 = s.send_message(msg.channel(), "wat");
			s.send_message(msg.channel(), "wat");
			auto m2 = s.send_message(msg.channel(), "wat");
			m2.cancel();
			auto m = co_await m1;			
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

	c.on_guild_msg_update = [](guild_msg_update g,shard& s) {
		if (g.content().has_value()) {
			const auto new_thing = g.content().value();
			std::cout << new_thing << std::endl;;
		}
	};

	c.on_guild_reaction_add = [](guild_member who, const text_channel& channel, snowflake msg_id, partial_emoji emoji, shard& s) ->cerwy::task<void> {
		if (who.id() == s.self_user().id()) {
			co_return;
		}
		const auto msg = co_await s.fetch_message(channel, msg_id);
		co_await s.delete_user_reaction(msg, emoji, who);
		co_await s.add_reaction(msg, emoji);


		boost::asio::steady_timer timer(s.strand());
		timer.expires_after(5s);
		co_await timer.async_wait(use_task);
		co_await s.delete_all_reactions(msg);
		co_return;
	};

	c.on_guild_reaction_remove = [](auto&&...) {
		std::cout << "reaction_removed" << std::endl;
	};

	c.on_guild_text_channel_create = [](const text_channel& channel,shard& s) ->cerwy::task<void>{
		co_await s.send_message(channel, "wat");
	};

	c.set_token(getFileContents("token.txt"));
	c.run();
	//} catch (...) {
	//std::cout << ";-;" << std::endl;
	//}
	std::cin.get();
	return 0;
}
