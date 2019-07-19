#include "voice_connect_impl.h"
#include "shard.h"
#include "web_socket_session_impl.h"
#include "discord_voice_connection.h"
#include "client.h"

cerwy::task<voice_connection> voice_connect_impl(shard& me,const voice_channel& ch,std::string endpoint, std::string token, std::string session_id) {
	const auto channel_id = ch.id();
	const auto guild_id = ch.guild_id();
	const auto my_id = me.self_user().id();
	//runs on strand until here
	auto web_socket = co_await create_session(endpoint, me.resolver(), me.ssl_context());
	//doesn't run on strand
	auto vc = make_ref_count_ptr<discord_voice_connection_impl>(std::move(web_socket));

	vc->channel_id = channel_id;
	vc->guild_id = guild_id;
	vc->my_id = my_id;
	vc->token = std::move(token);
	vc->endpoint = std::move(endpoint);
	vc->session_id = std::move(session_id);
	vc->heartbeat_sender = &me.parent_client().heartbeat_sender;

	cerwy::promise<void> p;
	vc->waiter = &p;
	vc->start();

	auto t = p.get_task();

	co_await t;
	vc->channel = &ch;
	//resume on strand, user code is resumed after this
	//idk how to do this better ;-;
	cerwy::promise<void> p2;
	auto tasky = p2.get_task();

	boost::asio::post(me.strand(), [&]() {
		p2.set_value();
	});

	co_await tasky;
	int put_breakpoint_here = 0;
	co_return voice_connection(std::move(vc));
}
