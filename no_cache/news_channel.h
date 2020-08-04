#pragma once
#include "text_channel.h"


namespace cacheless {
struct news_channel :text_channel {
private:
	inline friend void from_json(const nlohmann::json& json, news_channel& channel) {
		from_json(json, static_cast<text_channel&>(channel));


	}

};
}
