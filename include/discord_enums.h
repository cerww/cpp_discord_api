#pragma once
#include <string>
#include <string_view>

enum class Status {
	dnd,
	online,
	idle,
	invisible,
	offline
};

enum class event_name {
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
	GUILD_MEMBERS_CHUNK,
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
//TODO reorder these to mkae it chek more common ones first. Or use a trie
inline event_name to_event_name(std::string_view name) {
	if (name == "HELLO") return event_name::HELLO;
	if (name == "READY")return event_name::READY;
	if (name == "RESUMED")return event_name::RESUMED;
	if (name == "INVALID_SESSION")return event_name::INVALID_SESSION;
	if (name == "CHANNEL_CREATE")return event_name::CHANNEL_CREATE;
	if (name == "CHANNEL_UPDATE")return event_name::CHANNEL_UPDATE;
	if (name == "CHANNEL_DELETE")return event_name::CHANNEL_DELETE;
	if (name == "CHANNEL_PINES_UPDATE")return event_name::CHANNEL_PINS_UPDATE;
	if (name == "GUILD_CREATE")return event_name::GUILD_CREATE;
	if (name == "GUILD_UPDATE")return event_name::GUILD_UPDATE;
	if (name == "GUILD_DELETE")return event_name::GUILD_DELETE;
	if (name == "GUILD_BAN_ADD")return event_name::GUILD_BAN_ADD;
	if (name == "GUILD_BAN_REMOVE")return event_name::GUILD_BAN_REMOVE;
	if (name == "GUILD_EMOJI_UPDATE")return event_name::GUILD_EMOJI_UPDATE;
	if (name == "GUILD_INTEGRATIONS_UPDATE")return event_name::GUILD_INTEGRATIONS_UPDATE;
	if (name == "GUILD_MEMBER_ADD")return event_name::GUILD_MEMBER_ADD;
	if (name == "GUILD_MEMBER_REMOVE")return event_name::GUILD_MEMBER_REMOVE;
	if (name == "GUILD_MEMBER_UPDATE")return event_name::GUILD_MEMBER_UPDATE;
	if (name == "GUILD_MEMBERS_CHUNK")return event_name::GUILD_MEMBERS_CHUNK;
	if (name == "GUILD_ROLE_CREATE")return event_name::GUILD_ROLE_CREATE;
	if (name == "GUILD_ROLE_UPDATE")return event_name::GUILD_ROLE_UPDATE;
	if (name == "GUILD_ROLE_DELETE")return event_name::GUILD_ROLE_DELETE;
	if (name == "MESSAGE_CREATE")return event_name::MESSAGE_CREATE;
	if (name == "MESSAGE_UPDATE")return event_name::MESSAGE_UPDATE;
	if (name == "MESSAGE_DELETE")return event_name::MESSAGE_DELETE;
	if (name == "MESSAGE_DELETE_BULK")return event_name::MESSAGE_DELETE_BULK;
	if (name == "MESSAGE_REACTION_ADD")return event_name::MESSAGE_REACTION_ADD;
	if (name == "MESSAGE_REATION_REMOVE")return event_name::MESSAGE_REACTION_REMOVE;
	if (name == "MESSAGE_REACTION_REMOVE_ALL")return event_name::MESSAGE_REACTION_REMOVE_ALL;
	if (name == "PRESENCE_UPDATE")return event_name::PRESENCE_UPDATE;
	if (name == "TYPING_START")return event_name::TYPING_START;
	if (name == "USER_UPDATE")return event_name::USER_UPDATE;
	if (name == "VOICE_STATE_UPDATE")return event_name::VOICE_STATE_UPDATE;
	if (name == "VOICE_SERVER_UPDATE")return event_name::VOICE_SERVER_UPDATE;
	if (name == "WEBHOOKS_UPDATE")return event_name::WEBHOOKS_UPDATE;

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
		case Status::dnd:return "dnd";
		case Status::online: return "online";
		case Status::idle: return "idle";
		case Status::invisible: return "invisible";
		case Status::offline: return "offline";
		default: return "";
	}
}