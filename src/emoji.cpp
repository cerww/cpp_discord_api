#include "emoji.h"


snowflake partial_emoji::id() const noexcept { return m_id; }
std::string_view partial_emoji::name() const noexcept { return m_name; }
