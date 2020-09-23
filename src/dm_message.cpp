#include "dm_message.h"

const user& dm_message::author() const noexcept { return m_author; }

const dm_channel& dm_message::channel() const noexcept { return *m_channel; }

const std::optional<user>& dm_msg_update::author() const { return m_author; }

const dm_channel& dm_msg_update::channel() const noexcept { return *m_channel; }

