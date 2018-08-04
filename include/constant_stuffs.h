#pragma once
enum class Status {
	dnd,
	online,
	idle,
	invisible,
	offline
};

enum class eventName {
	HELLO,
	READY,
	RESUMED,
	INVALID_SESSION,
	CHANNEL_CREATE,
	CHANNEL_UPDATE,
	CHANNEL_DELETE,
	CHANNEL_PINS_UPDATE,
	GUILD_CREATE,
	GUILD_UPDATE,
	GUILD_DELETE,
	GUILD_BAN_ADD,
	GUILD_BAN_REMOVE,
	GUILD_EMOJI_UPDATE,
	GUILD_INTEGRATIONS_UPDATE,
	GUILD_MEMBER_ADD,
	GUILD_MEMBER_REMOVE,
	GUILD_MEMBER_UPDATE,
	GUILD_MEMBER_CHUNK,
	GUILD_ROLE_CREATE,
	GUILD_ROLE_UPDATE,
	GUILD_ROLE_DELETE,
	MESSAGE_CREATE,
	MESSAGE_UPDATE,
	MESSAGE_DELETE,
	MESSAGE_DELETE_BULK,
	MESSAGE_REACTION_ADD,
	MESSAGE_REACTION_REMOVE,
	MESSAGE_REACTION_REMOVE_ALL,
	PRESENCE_UPDATE,
	TYPING_START,
	USER_UPDATE,
	VOICE_STATE_UPDATE,
	VOICE_SERVER_UPDATE,
	WEBHOOKS_UPDATE
};

inline eventName to_event_name(const std::string& name) {
	if (name == "HELLO") return eventName::HELLO;
	if (name == "READY")return eventName::READY;
	if (name == "RESUMED")return eventName::RESUMED;
	if (name == "INVALID_SESSION")return eventName::INVALID_SESSION;
	if (name == "CHANNEL_CREATE")return eventName::CHANNEL_CREATE;
	if (name == "CHANNEL_UPDATE")return eventName::CHANNEL_UPDATE;
	if (name == "CHANNEL_DELETE")return eventName::CHANNEL_DELETE;
	if (name == "CHANNEL_PINES_UPDATE")return eventName::CHANNEL_PINS_UPDATE;
	if (name == "GUILD_CREATE")return eventName::GUILD_CREATE;
	if (name == "GUILD_UPDATE")return eventName::GUILD_UPDATE;
	if (name == "GUILD_DELETE")return eventName::GUILD_DELETE;
	if (name == "GUILD_BAN_ADD")return eventName::GUILD_BAN_ADD;
	if (name == "GUILD_BAN_REMOVE")return eventName::GUILD_BAN_REMOVE;
	if (name == "GUILD_EMOJI_UPDATE")return eventName::GUILD_EMOJI_UPDATE;
	if (name == "GUILD_INTEGRATIONS_UPDATE")return eventName::GUILD_INTEGRATIONS_UPDATE;
	if (name == "GUILD_MEMBER_ADD")return eventName::GUILD_MEMBER_ADD;
	if (name == "GUILD_MEMBER_REMOVE")return eventName::GUILD_MEMBER_REMOVE;
	if (name == "GUILD_MEMBER_UPDATE")return eventName::GUILD_MEMBER_UPDATE;
	if (name == "GUILD_MEMBERS_CHUNK")return eventName::GUILD_MEMBER_CHUNK;
	if (name == "GUILD_ROLE_CREATE")return eventName::GUILD_ROLE_CREATE;
	if (name == "GUILD_ROLE_UPDATE")return eventName::GUILD_ROLE_UPDATE;
	if (name == "GUILD_ROLE_DELETE")return eventName::GUILD_ROLE_DELETE;
	if (name == "MESSAGE_CREATE")return eventName::MESSAGE_CREATE;
	if (name == "MESSAGE_UPDATE")return eventName::MESSAGE_UPDATE;
	if (name == "MESSAGE_DELETE")return eventName::MESSAGE_DELETE;
	if (name == "MESSAGE_DELETE_BULK")return eventName::MESSAGE_DELETE_BULK;
	if (name == "MESSAGE_REACTION_ADD")return eventName::MESSAGE_REACTION_ADD;
	if (name == "MESSAGE_REATION_REMOVE")return eventName::MESSAGE_REACTION_REMOVE;
	if (name == "MESSAGE_REACTION_REMOVE_ALL")return eventName::MESSAGE_REACTION_REMOVE_ALL;
	if (name == "PRESENCE_UPDATE")return eventName::PRESENCE_UPDATE;
	if (name == "TYPING_START")return eventName::TYPING_START;
	if (name == "USER_UPDATE")return eventName::USER_UPDATE;
	if (name == "VOICE_STATE_UPDATE")return eventName::VOICE_STATE_UPDATE;
	if (name == "VOICE_SERVER_UPDATE")return eventName::VOICE_SERVER_UPDATE;
	if (name == "WEBHOOKS_UPDATE")return eventName::WEBHOOKS_UPDATE;

	//throw std::runtime_error(";-;");
	return {};

}
template<typename str_t>
inline Status string_to_status(str_t&& string) {
	if (string == "dnd") return Status::dnd;
	if (string == "online") return Status::online;
	if (string == "idle") return Status::idle;
	if (string == "invisible") return Status::invisible;
	if (string == "offline") return Status::offline;
	throw std::runtime_error("invalid string");
}

inline std::string enum_to_string(const Status s) {
	switch (s) {
	case Status::dnd:return "dnd"; break;
	case Status::online: return "online"; break;
	case Status::idle: return "idle"; break;
	case Status::invisible: return "invisible"; break;
	case Status::offline: return "offline"; break;
	default: return "";
	}
}