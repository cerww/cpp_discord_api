#include "guild_text_message.h"
#include "text_channel.h"



//const text_channel& guild_text_message::channel() const noexcept { return *m_channel; }

const guild_member& guild_text_message::author() const noexcept { return m_author; }

//const Guild& guild_text_message::guild() const noexcept { return channel().guild(); }

const std::optional<guild_member>& guild_msg_update::author() const noexcept { return m_author; }

const text_channel& guild_msg_update::channel() const noexcept { return *m_channel; }

