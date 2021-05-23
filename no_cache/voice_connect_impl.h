#pragma once
#include "discord_voice_connection.h"
#include "voice_channel.h"
#include "../common/eager_task.h"

namespace cacheless {
struct internal_shard;

//TODO: remove this along with init_shard
cerwy::eager_task<voice_connection> voice_connect_impl(internal_shard&, const voice_channel&, std::string gateway, std::string token, std::string session_id);

cerwy::eager_task<voice_connection> voice_connect_impl(internal_shard&, snowflake,snowflake, std::string gateway, std::string token, std::string session_id);

}
