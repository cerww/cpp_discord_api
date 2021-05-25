#include "include/client.h"
#include <fstream>
#include "include/modify_guild_settings.h"
#include "common/mp3_audio_source.h"
#include "common/usually_empty_vector.h"
#include "common/never_sso_string.h"
#include "common/big_uint.h"
#include <coroutine>
#include "common/lazy_task.h"
#include "common/cow_string.h"


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

cerwy::eager_task<void> do_audio_thingy(cerwy::eager_task<voice_connection> vc_task) {
	voice_connection channel = co_await vc_task;
	//co_await channel.send(audio_from_mp3("C:/Users/cerw/Downloads/a2.mp3"));
	co_await channel.send(mp3_audio_source(from_file{"C:/Users/cerw/Downloads/a.mp3"}));	
}

cerwy::eager_task<void> thingy(const Guild& g, shard& s) {
	while (true) {
		boost::asio::steady_timer timer(s.strand());
		timer.expires_after(20s);
		co_await timer.async_wait(use_task);
		//co_await resume_on_strand(s.strand());
		
		auto& role = g.roles()[snowflake(725880768659980330)];
		//s.send_message(s.text_channels().at(snowflake(504562059279728640)), role.to_mentionable_string() + " flag starts soon");
	}
}

void aujsdhasdasd() {
	std::vector<int> aaa = {12, 3, 4, 5, 67, 7, 8};
	usually_empty_vector<int> y = aaa | ranges::to<usually_empty_vector<int>>();
	assert(y[1] == aaa[1]);
	auto x = std::move(y);
	assert(y.size() == 0);
	usually_empty_vector<int> asdasd;
	assert(asdasd.size() == 0);
	assert(asdasd.empty());

	asdasd = usually_empty_vector<int>(aaa.begin(), aaa.end());
	assert(ranges::equal(aaa, asdasd));	

	std::span<int> wat = x;
	std::vector<int> c = wat | ranges::to<std::vector>();
	std::cout << (aaa == c) << std::endl;;

	
}


void test_never_sso_string() {
	never_sbo_string s1;
	s1 = "aaaa";
	std::string s1_real_str = "aaaa";
}

cerwy::eager_task<void> test_queue1(async_queue_maybe_better<int>& queue) {
	for (int i = 0; i < 20001; ++i) {
		const int a = co_await queue.pop();
		//assert(a == i);
	}
	std::cout << "bbb" << std::endl;
}

cerwy::eager_task<void> test_new_queue(async_queue_maybe_better<int>& queue) {

	for (int i = 0; i < 10000; ++i) {
		queue.push(i);
	}

	co_return;
}

void test_new_queuea() {
	async_queue_maybe_better<int> queue;
	auto t = std::jthread([&]() {
		test_new_queue(queue);
	});

	auto t2 = std::jthread([&]() {
		test_new_queue(queue);
	});

	test_queue1(queue);

	std::cout << "aaa" << std::endl;
	std::cin.get();
}

void test_big_int2() {
	for (uint64_t i = 0; i < 1000; ++i) {
		auto e = big_uint(i + (uint64_t)std::numeric_limits<uint32_t>::max());
		auto k = big_uint(std::to_string(i + (uint64_t)std::numeric_limits<uint32_t>::max()));
		assert(e == k);
		assert(e == (i + (uint64_t)std::numeric_limits<uint32_t>::max()));
		auto qwe = big_uint(i);
		qwe += (uint64_t)std::numeric_limits<uint32_t>::max();
		assert(qwe == e);
	}
	auto a = big_uint("54444");
	auto b = big_uint(std::numeric_limits<uint32_t>::max());
	assert(b == std::numeric_limits<uint32_t>::max());
	b += b;
	assert(b == (uint64_t)std::numeric_limits<uint32_t>::max()*2);
	assert(b == big_uint((uint64_t)std::numeric_limits<uint32_t>::max() * 2));
	assert(a == 54444);
	auto c = big_uint(std::numeric_limits<int>::max());
	c *= c;
	assert(c == (uint64_t)std::numeric_limits<int>::max()* (uint64_t)std::numeric_limits<int>::max());
	auto d = big_uint("4611686014132420609");
	assert(c == d);
	
}

//https://keisan.casio.com/calculator
//68719476735

void test_big_int1() {
	test_big_int2();
	auto a = big_uint("4722366482732206260225");
	auto b = big_uint("4722366482732206260224");
	auto d = big_uint("9444732965464412520448");
	auto c = big_uint(68719476735ull);
	c *= c;	
	assert(b + 1 == a);
	assert(a - 1 == b);
	assert(a == c);
	assert(b * 2 == d);
	assert(b != d);
	assert(a != b);
	
}

void test_big_int() {
	test_big_int1();
	big_uint a;
	auto b = a + 1;
	a += 2;
	a -= 2;
	assert(a == 0);
	assert(b == 1);
	assert(b * a == 0);
	b = 0;
	assert(a == b);
	a += 5;
	assert(a == 5);
	a = 6ull;
	//std::cout << to_string(a) << std::endl;
	assert(a == 6);
	assert(a != 50);
	a = 0xfffffffffull;
	assert(a == 0xfffffffffull);
	std::cout << to_string(a) << std::endl;
	const auto a_squared = a * a;
	a *= a;
	assert(a_squared == a);
	auto qwee = big_uint("1234567890101112");
	std::cout << to_string(qwee) << std::endl;
	qwee *= qwee;
	std::cout << to_string(qwee) << std::endl;
	qwee = 1 * std::move(qwee);
	assert(qwee == big_uint("1524157875268711356997583636544"));
	assert(qwee != big_uint("15241578752687113569975836365441"));
	
	//assert(a == 0);
	std::cout << "done big_uint tests ;-;" << std::endl;	
}

void test_big_uint_bug() {
	auto qwee = big_uint("1234567890101112");
	std::cout << to_string(qwee) << std::endl;
	qwee *= qwee;
	std::cout << to_string(qwee) << std::endl;
	qwee = 1 * std::move(qwee);
	assert(qwee == big_uint("1524157875268711356997583636544"));
	assert(qwee != big_uint("15241578752687113569975836365441"));
}

cerwy::eager_task<int> watland123() {
	co_return 123;
}

cerwy::lazy_task<int> lazy_task_test2() {
	co_return co_await watland123();
}

cerwy::lazy_task<int> lazy_task_test() {
	const int a = co_await lazy_task_test2();
	std::cout << "b" << a << std::endl;
	co_return 1;
}

cerwy::eager_task<void> test_lazy_task_thing() {
	const auto a = lazy_task_test();
	std::cout << "a" << std::endl;
	std::cout << co_await a << std::endl;
}

void test_cow_string() {
	cow_string rawr = "wat";
	auto c = rawr;

	std::cout << (&c[0] == &rawr[0]) << std::endl;
	rawr = cow_string("bonkland"sv);


	
	
	std::cout << std::string_view(c) << std::endl;
	std::cout << (c == "wat") << std::endl;
	
}

//spam bot
int main() {

	test_cow_string();
	//std::cin.get();
	//test_lazy_task_thing();
	//test_big_uint_bug();
	//setlocale(LC_ALL, "");
	//test_big_int();
	//std::cin.get();
	

	// aujsdhasdasd();
	// std::cin.get();

	
	client c;

	cerwy::eager_task<voice_connection> connashk;
	std::vector<partial_message> msgs;

	c.on_guild_ready = [&](const Guild& g, shard& s) {
		if (g.id() == snowflake(199218706029608961)) {
			//thingy(g, s);
		}
	};

	//c.on_guild_ready = &thingy;
	
	c.on_guild_text_msg = [&](guild_text_message msg, shard& s)->cerwy::eager_task<void> {
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
					ranges::views::transform([](std::string_view s) {if (s.starts_with('@')) return s.substr(1); else return s; }) |
					ranges::views::join(" "sv) |
					ranges::to<std::string>();

			s.send_message(msg.channel(), std::move(stuff), disable_mentions).execute_and_ignore();
		} else if (msg.content() == "invite") {
			//co_await s.create_channel_invite(msg.channel());
		} else if (msg.content() == "namey") {
			s.send_message(msg.channel(), std::string(msg.author().nick())).execute_and_ignore();
		} else if (msg.content() == ";-;worldlandasdasdasd") {
			const auto role = msg.guild().role_by_name("Prophets of Milktea-ism");
			if (role.has_value()) {
				s.send_message(msg.channel(), role->to_mentionable_string(), disable_mentions).execute_and_ignore();
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
							   embed_field().set_is_inline(true).set_name("better field").set_value("awesome"),
							   embed_field().set_is_inline(true).set_name("more awesome").set_value("best field")
						   )
			).execute_and_ignore();
		} else if (msg.content() == "hamtaroland_worldy") {
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
			s.send_message(msg.channel(), "charmander").execute_and_ignore();
		} else if (msg.content() == "at everyone") {
			s.send_message(msg.channel(), "@everyone", allowed_mentions()).execute_and_ignore();
		} else if (msg.content() == "at petery") {
			s.send_message(msg.channel(), "<@188547243911938048>", disable_mentions).execute_and_ignore();
		} else if (msg.content() == "hello kitty") {
			const auto new_msg = co_await s.send_message(msg.channel(), "charmanderworld");
			std::cout << new_msg.content() << std::endl;
			//std::cout << new_msg.author().id().val << std::endl;
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
			std::fstream file = std::fstream("abcd.txt", std::ios::trunc);

			file << str;
			file.close();

		} else if (msg.content() == "charizard_world") {
			//dropped cancelation support
			auto m1 = s.send_message(msg.channel(), "wat");
			s.send_message(msg.channel(), "wat").execute_and_ignore();
			auto m2 = s.send_message(msg.channel(), "wat");
			//m2.cancel();
			auto m = co_await m1;
		}else if(msg.content() == "fyy") {
			//auto a = L"後生仔傾吓偈"s;
			
			//s.send_message(msg.channel(), ).execute_and_ignore();
		}
		//s.change_nick(wat.author(), wat.content());


		for (const auto& i : msg.mentions()) {
			//s.change_nick(i, std::string(msg.content())).execute_and_ignore();
		}

		if (msg.author().id() != s.self_user().id()) {
			s.send_message(msg.channel(), std::to_string(msg.author().id().val)).execute_and_ignore();
		}
		//s.add_reaction(wat,wat.guild().emojis().back());
		if(msg.referenced_message()) {
			std::cout << msg.referenced_message().value().author().id().as_int() << std::endl;;
			std::cout << msg.referenced_message().value().content() << std::endl;
			s.send_message(msg.channel(), std::string(msg.referenced_message().value().content())).execute_and_ignore();
		}

	};
	c.on_guild_typing_start = [&](guild_member member, const text_channel& channel, shard& s) {
		s.send_message(channel, "rawr").execute_and_ignore();
		std::cout << "rawr" << std::endl;		
	};

	c.on_guild_member_add = [&](const guild_member& member, shard& s) {
		s.send_message(member.guild().system_channel(), "rawr").execute_and_ignore();
		//s.modify_guild(member.guild(), guild_settings::default_message_notifications{1});
		//auto t = modify_guild_settings().name("aaa");
	};

	c.on_guild_msg_update = [](guild_msg_update g,shard& s) {
		if (g.content().has_value()) {
			const std::string_view new_thing = g.content().value();
			std::cout << new_thing << std::endl;;
		}
	};

	c.on_guild_reaction_add = [](guild_member who, const text_channel& channel, snowflake msg_id, partial_emoji emoji, shard& s) ->cerwy::eager_task<void> {
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

	c.on_guild_text_channel_create = [](const text_channel& channel,shard& s) ->cerwy::eager_task<void> {
		(void)co_await s.send_message(channel, "wat");
	};

	c.set_token(getFileContents("token.txt"));
	c.run();
	
	//} catch (...) {
	//std::cout << ";-;" << std::endl;
	//}
	
	std::cin.get();
	return 0;
}
