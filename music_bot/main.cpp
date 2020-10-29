#include "../include/client.h"
#include "../command_parser/command_context.h"
#include <fstream>
#include "../common/ytdl_audio_source.h"


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

cerwy::task<void> play_stuffs(voice_connection& vc, async_queue_thread_unsafe<std::string>& queue) {
	while (vc.is_connected()) {
		auto next_query = co_await queue.pop();
		co_await vc.send_async(ytdl_search_source(std::move(next_query), vc.strand().context()));			
	}
}

struct connection_stuffs {
	connection_stuffs() = default;

	explicit connection_stuffs(voice_connection v):
		vc(std::move(v)) {}

	voice_connection vc;
	async_queue_thread_unsafe<std::string> queue;
	cerwy::task<void> thingy;
};

//wat is asio.ssl.337690831
//#include <openssl/err.h>


int main() {
	const std::string err = std::string(" (")
		+ std::to_string(ERR_GET_LIB(337690831)) + ","
		+ std::to_string(ERR_GET_FUNC(337690831)) + ","
		+ std::to_string(ERR_GET_REASON(337690831)) + ") ";
	std::cout << err << std::endl;
	client c;
	c.set_token(getFileContents("token.txt"));

	shiny::command_context command_thingy;
	command_thingy.prefix = "!!!!";

	std::unordered_map<snowflake, connection_stuffs> connections;

	command_thingy["join"] += shiny::make_command([&](guild_text_message m,shard& s) ->cerwy::task<void> {
		const auto channel_ref = m.guild().voice_channel_for(m.author());
		if (channel_ref.has_value()) {
			auto connection = co_await s.connect_voice(channel_ref.value());
			connections[m.guild().id()].vc = std::move(connection);
			connections[m.guild().id()].thingy = play_stuffs(connections[m.guild().id()].vc, connections[m.guild().id()].queue);			
		} else {
			co_await s.send_reply(m, "not connected");
		}
	});

	command_thingy["quit"] += shiny::make_command([&](guild_text_message m,shard& s) ->cerwy::unawaitable_action{
		if (connections.contains(m.guild().id())) {
			auto& thing = connections[m.guild().id()];
			thing.queue.cancel_all();
			thing.vc.disconnect();
			co_await connections[m.guild().id()].thingy;
			connections.erase(m.guild().id());
		} else {
			co_await s.send_reply(m, "wat");
		}
	});

	command_thingy["play"] += shiny::make_command<std::string>([&](std::string query,guild_text_message m,shard& s) {
		if (connections.contains(m.guild().id())) {
			connections[m.guild().id()].queue.push(std::string(query));
		} else {
			s.send_reply(m, "not connected").execute_and_ignore();
		}
	});

	command_thingy["show_queue"] += shiny::make_command([&](guild_text_message m,shard& s) {
		if (connections.contains(m.guild().id())) {
			auto stuff = connections[m.guild().id()].queue.data();
			s.send_reply(m, connections[m.guild().id()].queue.data() | ranges::views::join('\n') | ranges::to<std::string>()).execute_and_ignore();
		} else {
			s.send_reply(m, "not connected").execute_and_ignore();
		}
	});

	command_thingy["say"] += shiny::make_command<std::string>([](std::string wat,guild_text_message m, shard& s) {
		s.send_reply(m, wat).execute_and_ignore();
	});

	c.on_guild_text_msg = hof::bind1st(&shiny::command_context::do_command, command_thingy);

	c.run();
}
