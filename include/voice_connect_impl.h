#pragma once
#include "task.h"
#include "discord_voice_connection.h"
#include "voice_channel.h"

struct shard;

//TODO: remove this along with init_shard
cerwy::task<voice_connection> voice_connect_impl(shard&, const voice_channel&, std::string gateway, std::string token, std::string session_id);

