#include "emoji.h"


snowflake partial_emoji::id() const noexcept { return m_id; }
const std::string& partial_emoji::name() const noexcept { return m_name; }
