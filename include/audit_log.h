#pragma once
#include "webhook.h"
#include "guild_integration.h"
#include <variant>
#include "permision_overwrite.h"
#include <span>
#include <unordered_set>

enum class audit_log_entry_type {
	GUILD_UPDATE = 1,
	CHANNEL_CREATE = 10,
	CHANNEL_UPDATE = 11,
	CHANNEL_DELETE = 12,
	CHANNEL_OVERWRITE_CREATE = 13,
	CHANNEL_OVERWRITE_UPDATE = 14,
	CHANNEL_OVERWRITE_DELETE = 15,
	MEMBER_KICK = 20,
	MEMBER_PRUNE = 21,
	MEMBER_BAN_ADD = 22,
	MEMBER_BAN_REMOVE = 23,
	MEMBER_UPDATE = 24,
	MEMBER_ROLE_UPDATE = 25,
	MEMBER_MOVE = 26,
	MEMBER_DISCONNECT = 27,
	BOT_ADD = 28,
	ROLE_CREATE = 30,
	ROLE_UPDATE = 31,
	ROLE_DELETE = 32,
	INVITE_CREATE = 40,
	INVITE_UPDATE = 41,
	INVITE_DELETE = 42,
	WEBHOOK_CREATE = 50,
	WEBHOOK_UPDATE = 51,
	WEBHOOK_DELETE = 52,
	EMOJI_CREATE = 60,
	EMOJI_UPDATE = 61,
	EMOJI_DELETE = 62,
	MESSAGE_DELETE = 72,
	MESSAGE_BULK_DELETE = 73,
	MESSAGE_PIN = 74,
	MESSAGE_UNPIN = 75,
	INTEGRATION_CREATE = 80,
	INTEGRATION_UPDATE = 81,
	INTEGRATION_DELETE = 82,
};

//in order to keep the size of audit_log_change, small, if this wasn't heap allocated, this would use 160 bytes total, most of it unused
//since 2nd largest thing is std::string, with size == 32=>64 bytes used=>96 bytes wasted=> over 100 bytes wasted avg prolly
struct changed_role_impl {
	snowflake id() const noexcept { return m_id; }

	std::optional<std::string_view> name() const noexcept { return m_name; }

	std::optional<int> color() const noexcept { return m_color == -1 ? std::optional<int>() : m_color; }

	std::optional<int> position() const noexcept { return m_position == -1 ? std::optional<int>() : m_position; }

	std::optional<permission> permissions() const noexcept { return m_permissions; }

	std::optional<bool> managed() const noexcept { return m_managed; }

	std::optional<bool> hoist() const noexcept { return m_hoist; }

	std::optional<bool> mentionable() const noexcept { return m_mentionable; }

private:
	std::optional<std::string> m_name;
	std::optional<permission> m_permissions;
	snowflake m_id;
	int m_color = -1;
	int m_position = -1;
	std::optional<bool> m_hoist;
	std::optional<bool> m_managed;
	std::optional<bool> m_mentionable;

	friend void from_json(const nlohmann::json& json, changed_role_impl& role) {
		role.m_name = json.value("name", std::optional<std::string>());
		role.m_permissions = json.value("name", std::optional<permission>());
		role.m_id = json.value("name", snowflake());
		role.m_color = json.value("name", -1);
		role.m_position = json.value("name", -1);
		role.m_hoist = json.value("name", std::optional<bool>());
		role.m_managed = json.value("name", std::optional<bool>());
		role.m_mentionable = json.value("name", std::optional<bool>());
	}
};

struct changed_role {

	snowflake id() const noexcept { return m_me->id(); }

	std::optional<std::string_view> name() const noexcept { return m_me->name(); }

	std::optional<int> color() const noexcept { return m_me->color(); }

	std::optional<int> position() const noexcept { return m_me->position(); }

	std::optional<permission> permissions() const noexcept { return m_me->permissions(); }

	std::optional<bool> managed() const noexcept { return m_me->managed(); }

	std::optional<bool> hoist() const noexcept { return m_me->hoist(); }

	std::optional<bool> mentionable() const noexcept { return m_me->mentionable(); }

private:

	indirect<changed_role_impl> m_me;

	friend void from_json(const nlohmann::json& json, changed_role& role) {
		role.m_me = json.get<changed_role_impl>();
	}
};

static constexpr int yiyqweeuhjasd = sizeof(changed_role_impl);

struct audit_log_change {
	using changed_type_type = std::variant<int, std::string, snowflake, std::vector<permission_overwrite>, bool, changed_role>;

	const auto& old_value() const noexcept { return m_old_value; }

	const auto& new_value() const noexcept { return m_new_value; }

	std::string_view key() const noexcept { return m_key; }

	inline static const std::unordered_set<std::string_view> keys_that_are_int = {
		"afk_timeout", "mfa_level", "verification_level", "explicit_content_filter", "default_message_notification",
		"prune_delete_days", "position", "bitrate", "rate_limit_per_user", "permissions", "color",
		"allow", "deny", "max_uses", "uses", "max_age",
	};//"type" is int or string, not included here

	inline static const std::unordered_set<std::string> keys_that_are_strings = {
		"name", "icon_hash", "splash_hash", "region", "vanity_url_code", "topic", "code", "nick", "avatar_hash"
	};

	inline static const std::unordered_set<std::string> keys_that_are_bools = {
		"widget_enabled", "nsfw", "hoist", "mentionable", "mentionable", "deaf", "mute", "enable_emoticons"
	};

	inline static const std::unordered_set<std::string> keys_that_are_snowflakes = {
		"owner_id", "afk_channel_id", "widget_channel_id", "system_channel_id", "application_id", "channel_id", "inviter_id", "id"
	};

private:
	changed_type_type m_old_value;
	changed_type_type m_new_value;
	std::string m_key;

	friend void from_json(const nlohmann::json& json, audit_log_change& log_change) {
		log_change.m_key = json["key"].get<std::string>();


		if (keys_that_are_int.contains(log_change.m_key)) {
			log_change.m_old_value = json.value("old_value", 0);
			log_change.m_new_value = json.value("new_value", 0);
		} else if (keys_that_are_bools.contains(log_change.m_key)) {
			log_change.m_old_value = json.value("old_value", false);
			log_change.m_new_value = json.value("new_value", false);
		} else if (keys_that_are_snowflakes.contains(log_change.m_key)) {
			log_change.m_old_value = json.value("old_value", snowflake(0));
			log_change.m_new_value = json.value("new_value", snowflake(0));
		} else if (keys_that_are_strings.contains(log_change.m_key)) {
			log_change.m_old_value = json.value("old_value", "");
			log_change.m_new_value = json.value("new_value", "");
		} else if (log_change.m_key == "permission_overwrites") {
			log_change.m_old_value = json.value("old_value", std::vector<permission_overwrite>());
			log_change.m_new_value = json.value("new_value", std::vector<permission_overwrite>());
		} else if (log_change.m_key == "$add" || log_change.m_key == "$remove") {
			log_change.m_old_value = json.value("old_value", changed_role());
			log_change.m_new_value = json.value("new_value", changed_role());
		} else if (log_change.m_key == "type") {
			const auto new_value_it = json.find("new_value");
			if (new_value_it != json.end()) {
				if (new_value_it->is_string()) {
					log_change.m_new_value = new_value_it->get<std::string>();
				}
			}
			const auto old_value_it = json.find("new_value");
			if (old_value_it != json.end()) {
				if (old_value_it->is_string()) {
					log_change.m_new_value = old_value_it->get<std::string>();
				}
			}
		}
	}
};

static constexpr int yiyqweeuhjasdaaa = sizeof(audit_log_change);

struct additional_entry_info {
	std::optional<std::string_view> delete_member_days() const noexcept {
		return m_delete_member_days;
	}

	std::optional<std::string_view> members_removed() const noexcept {
		return m_members_removed;
	}

	std::optional<snowflake> channel_id() const noexcept {
		return m_channel_id;
	}

	std::optional<snowflake> message_id() const noexcept {
		return m_message_id;
	}

	std::optional<std::string_view> count() const noexcept {
		return m_count;
	}

	std::optional<snowflake> id() const noexcept {
		return m_id;
	}

	std::optional<std::string_view> type() const noexcept {
		return m_type;
	}

	std::optional<std::string_view> role_name() const noexcept {
		return m_role_name;
	}

private:

	std::optional<std::string> m_delete_member_days;
	std::optional<std::string> m_members_removed;
	std::optional<snowflake> m_channel_id;
	std::optional<snowflake> m_message_id;
	std::optional<std::string> m_count;
	std::optional<snowflake> m_id;
	std::optional<std::string> m_type;
	std::optional<std::string> m_role_name;


	friend void from_json(const nlohmann::json& json, additional_entry_info& info) {
		info.m_delete_member_days = json.value("delete_member_days", std::optional<std::string>());
		info.m_members_removed = json.value("members_removed", std::optional<std::string>());
		info.m_channel_id = json.value("channel_id", snowflake());
		info.m_message_id = json.value("message_id", snowflake());
		info.m_count = json.value("count", std::optional<std::string>());
		info.m_id = json.value("id", snowflake(0));
		info.m_type = json.value("type", std::optional<std::string>());
		info.m_role_name = json.value("role_name", std::optional<std::string>());

	}
};

struct audit_log_entry {
	std::optional<snowflake> target_id() const noexcept { return m_id; }

	std::span<const audit_log_change> changes() const noexcept { return m_changes; }

	snowflake user_id() const noexcept { return m_user_id; }

	snowflake id() const noexcept { return m_id; }

	audit_log_entry_type action_type() const noexcept { return m_action_type; }

	const std::optional<additional_entry_info>& options() const noexcept { return m_options; }

	std::optional<std::string_view> reason() const noexcept { return m_reason; }

private:
	std::optional<snowflake> m_target_id;
	std::vector<audit_log_change> m_changes;
	snowflake m_user_id;
	snowflake m_id;
	audit_log_entry_type m_action_type{};
	std::optional<additional_entry_info> m_options;
	std::optional<std::string> m_reason;


	friend void from_json(const nlohmann::json& json, audit_log_entry& entry) {
		entry.m_target_id = json.value("target_id", snowflake(0));
		entry.m_changes = json.value("changes", std::vector<audit_log_change>());
		entry.m_user_id = json["user_id"].get<snowflake>();
		entry.m_id = json["id"].get<snowflake>();
		entry.m_action_type = json["action_type"].get<audit_log_entry_type>();
		entry.m_options = json.value("options", std::optional<additional_entry_info>());
		entry.m_reason = json.value("reason", std::optional<std::string>());
		
		//additional_entry_info a = std::optional<additional_entry_info>();
	}
};

struct audit_log {
	std::span<const webhook> webhooks() const noexcept {
		return m_webhooks;
	}

	std::span<const user> users() const noexcept {
		return m_users;
	}

	std::span<const partial_integration> integrations() const noexcept {
		return m_integrations;
	}

	std::span<const audit_log_entry> entries() const noexcept {
		return m_audit_log_entries;
	}

private:
	std::vector<webhook> m_webhooks;
	std::vector<user> m_users;
	std::vector<partial_integration> m_integrations;
	std::vector<audit_log_entry> m_audit_log_entries;

	friend void from_json(const nlohmann::json& json, audit_log& logs) {
		logs.m_webhooks = json["webhooks"].get<std::vector<webhook>>();
		logs.m_users = json["users"].get<std::vector<user>>();
		logs.m_integrations = json["integrations"].get<std::vector<partial_integration>>();
		logs.m_audit_log_entries = json["audit_log_entries"].get<std::vector<audit_log_entry>>();
	}
};
