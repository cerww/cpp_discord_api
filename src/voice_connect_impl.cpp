#include "voice_connect_impl.h"
#include "shard.h"
#include "web_socket_session_impl.h"
#include "discord_voice_connection.h"
#include "client.h"
#include "resume_on_strand.h"

using namespace boost::asio;

cerwy::task<voice_connection> voice_connect_impl(shard& me, const voice_channel& ch, std::string endpoint, std::string token, std::string session_id) {
	const auto channel_id = ch.id();
	const auto guild_id = ch.guild_id();
	const auto my_id = me.self_user().id();
	//runs on strand until here
	auto web_socket = co_await create_session(endpoint, me.strand().context(), ssl::context_base::method::sslv23);
	//doesn't run on strand
	auto vc = make_ref_count_ptr<discord_voice_connection_impl>(std::move(web_socket),me.parent_client().context());

	vc->channel_id = channel_id;
	vc->guild_id = guild_id;
	vc->my_id = my_id;
	vc->token = std::move(token);
	vc->web_socket_endpoint = std::move(endpoint);
	vc->session_id = std::move(session_id);
	vc->strand = &me.strand();
	//vc->heartbeat_sender = &me.parent_client().heartbeat_sender;

	cerwy::promise<void> p;
	vc->waiter = &p;
	vc->start();

	auto t = p.get_task();

	co_await t;//wait till it's done setting up
	vc->channel = &ch;
	co_await resume_on_strand{me.strand()};
	int put_breakpoint_here = 0;
	co_return voice_connection(std::move(vc));
}
