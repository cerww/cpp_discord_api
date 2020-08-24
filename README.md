# cpp discord api
===============

discord api for c++

-supports voice

# Dependancies

-Boost

-Rangev3

-nlohmann json

-fmt

-c++20 coroutines

-C++20 Concepts

-minimp3

-libopus

-ffmpeg and youtube-dl needed in path for ytdl support

https://youtu.be/RrGX7g9xsY0 for working example


# usage

```C++

#include "include/client.h"

int main() {
	client c;
	c.on_guild_text_msg = [](const guild_text_message& msg, shard& s) {
		if(msg.content() == "ping") {
			s.send_message(msg.channel(),"pong");
		}
	};
	
	c.set_token("TOKEN",token_type::BOT);
	
	c.run();
	return 0;
}

```