#include "voice_connect_impl.h"
#include "internal_shard.h"
#include "../common/web_socket_session_impl.h"
#include "discord_voice_connection.h"
#include "client.h"
#include "../common/resume_on_strand.h"

using namespace boost::asio;

cerwy::eager_task<std::pair<voice_connection,ref_count_ptr<discord_voice_connection_impl>>> voice_connect_impl(internal_shard& me, const voice_channel& ch, std::string endpoint, std::string token, std::string session_id) {
	const auto channel_id = ch.id();
	const auto guild_id = ch.guild_id();
	const auto my_id = me.self_user().id();
	ref_count_ptr<const Guild> pin_guild = &ch.guild();
	//runs on strand until here
	auto web_socket = co_await create_session(endpoint, me.strand().context().get_executor(), ssl::context_base::method::sslv23);
	//doesn't run on strand
	auto vc = make_ref_count_ptr<discord_voice_connection_impl>(std::move(web_socket),me.strand().context());
	
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
	vc->guild = std::move(pin_guild);
	co_await resume_on_strand{me.strand()};
	int put_breakpoint_here = 0;
	auto vc_copy = vc;//so I only increment the ref count once!!!111!1!!
	co_return std::pair(voice_connection(std::move(vc)), std::move(vc_copy));
}
