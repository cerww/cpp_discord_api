#pragma once
#include "discord_enums.h"

/*
GUILDS (1 << 0)
  - GUILD_CREATE
  - GUILD_UPDATE
  - GUILD_DELETE
  - GUILD_ROLE_CREATE
  - GUILD_ROLE_UPDATE
  - GUILD_ROLE_DELETE
  - CHANNEL_CREATE
  - CHANNEL_UPDATE
  - CHANNEL_DELETE
  - CHANNEL_PINS_UPDATE

GUILD_MEMBERS (1 << 1)
  - GUILD_MEMBER_ADD
  - GUILD_MEMBER_UPDATE
  - GUILD_MEMBER_REMOVE

GUILD_BANS (1 << 2)
  - GUILD_BAN_ADD
  - GUILD_BAN_REMOVE

GUILD_EMOJIS (1 << 3)
  - GUILD_EMOJIS_UPDATE

GUILD_INTEGRATIONS (1 << 4)
  - GUILD_INTEGRATIONS_UPDATE

GUILD_WEBHOOKS (1 << 5)
  - WEBHOOKS_UPDATE

GUILD_INVITES (1 << 6)
  - INVITE_CREATE
  - INVITE_DELETE

GUILD_VOICE_STATES (1 << 7)
  - VOICE_STATE_UPDATE

GUILD_PRESENCES (1 << 8)
  - PRESENCE_UPDATE

GUILD_MESSAGES (1 << 9)
  - MESSAGE_CREATE
  - MESSAGE_UPDATE
  - MESSAGE_DELETE
  - MESSAGE_DELETE_BULK

GUILD_MESSAGE_REACTIONS (1 << 10)
  - MESSAGE_REACTION_ADD
  - MESSAGE_REACTION_REMOVE
  - MESSAGE_REACTION_REMOVE_ALL
  - MESSAGE_REACTION_REMOVE_EMOJI

GUILD_MESSAGE_TYPING (1 << 11)
  - TYPING_START

DIRECT_MESSAGES (1 << 12)
  - CHANNEL_CREATE
  - MESSAGE_CREATE
  - MESSAGE_UPDATE
  - MESSAGE_DELETE
  - CHANNEL_PINS_UPDATE

DIRECT_MESSAGE_REACTIONS (1 << 13)
  - MESSAGE_REACTION_ADD
  - MESSAGE_REACTION_REMOVE
  - MESSAGE_REACTION_REMOVE_ALL
  - MESSAGE_REACTION_REMOVE_EMOJI

DIRECT_MESSAGE_TYPING (1 << 14)
  - TYPING_START

*/


/*
enum class intents :uint32_t {
	GUILDS = (1 << 0),
	GUILD_MEMBERS=(1 << 1),
	GUILD_BANS=(1 << 2),
	GUILD_EMOJIS=(1 << 3),
	GUILD_INTEGRATIONS=(1 << 4),
	GUILD_WEBHOOKS=(1 << 5),
	GUILD_INVITES=(1 << 6),
	GUILD_VOICE_STATES=(1 << 7),
	GUILD_PRESENCES=(1 << 8),
	GUILD_MESSAGES=(1 << 9),
	GUILD_MESSAGE_REACTIONS=(1 << 10),
	GUILD_MESSAGE_TYPING=(1 << 11),
	DIRECT_MESSAGES=(1 << 12),
	DIRECT_MESSAGE_REACTIONS=(1 << 13),
	DIRECT_MESSAGE_TYPING = (1 << 14),
	ALL = (GUILDS | GUILD_MEMBERS | GUILD_BANS | GUILD_EMOJIS | GUILD_INTEGRATIONS | GUILD_WEBHOOKS | GUILD_INVITES
		| GUILD_VOICE_STATES | GUILD_PRESENCES | GUILD_MESSAGES | GUILD_MESSAGE_REACTIONS
		| GUILD_MESSAGE_TYPING | DIRECT_MESSAGES | DIRECT_MESSAGE_REACTIONS | DIRECT_MESSAGE_TYPING),
	ALL_BUT_PRESSENCE_MEMBERS = ~(~ALL | GUILD_MEMBERS | GUILD_PRESENCES)
};
*/

//not an enum so i can have functions 
struct intents {
	constexpr intents() = default;

	constexpr explicit intents(uint32_t value) :
		m_value(value) { }

	constexpr bool has_intents(intents other) const noexcept {
		return (m_value & other.m_value) == other.m_value;
	}

	constexpr friend intents operator|(intents me, intents other) {
		return intents(me.m_value | other.m_value);
	}

	constexpr friend intents operator+(intents me, intents other) {
		return intents(me.m_value | other.m_value);
	}

	constexpr intents removed(intents other) const noexcept {
		return intents(~(~m_value | other.m_value));
	}

	constexpr friend intents operator-(intents me, intents other) {
		return me.removed(other);
	}

	uint32_t int_value()const noexcept {
		return m_value;
	}	

private:
	uint32_t m_value = 0;
};

template<typename ...intents_v>
constexpr intents combine_intents(intents_v ... intents) {
	return (intents | ...);
}


namespace intent {
	static constexpr intents GUILDS = intents(1 << 0);
	static constexpr intents GUILD_MEMBERS = intents(1 << 1);
	static constexpr intents GUILD_BANS = intents(1 << 2);
	static constexpr intents GUILD_EMOJIS = intents(1 << 3);
	static constexpr intents GUILD_INTEGRATIONS = intents(1 << 4);
	static constexpr intents GUILD_WEBHOOKS = intents(1 << 5);
	static constexpr intents GUILD_INVITES = intents(1 << 6);
	static constexpr intents GUILD_VOICE_STATES = intents(1 << 7);
	static constexpr intents GUILD_PRESENCES = intents(1 << 8);
	static constexpr intents GUILD_MESSAGES = intents(1 << 9);
	static constexpr intents GUILD_MESSAGE_REACTIONS = intents(1 << 10);
	static constexpr intents GUILD_MESSAGE_TYPING = intents(1 << 11);
	static constexpr intents DIRECT_MESSAGES = intents(1 << 12);
	static constexpr intents DIRECT_MESSAGE_REACTIONS = intents(1 << 13);
	static constexpr intents DIRECT_MESSAGE_TYPING = intents(1 << 14);
	static constexpr intents ALL = (GUILDS | GUILD_MEMBERS | GUILD_BANS | GUILD_EMOJIS | GUILD_INTEGRATIONS | GUILD_WEBHOOKS | GUILD_INVITES
		| GUILD_VOICE_STATES | GUILD_PRESENCES | GUILD_MESSAGES | GUILD_MESSAGE_REACTIONS
		| GUILD_MESSAGE_TYPING | DIRECT_MESSAGES | DIRECT_MESSAGE_REACTIONS | DIRECT_MESSAGE_TYPING);

	static constexpr intents ALL_BUT_PRESSENCE_MEMBERS = ALL - GUILD_MEMBERS - GUILD_PRESENCES;;
	static constexpr intents ALL_BUT_TYPING = ALL - GUILD_MESSAGE_TYPING - DIRECT_MESSAGE_TYPING;
}