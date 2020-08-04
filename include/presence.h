#pragma once
#include "presence_update.h"


struct presence {
	presence() = default;
	
	explicit presence(const partial_presence_update&) {
		
	}
	
	const std::optional<activity>& activity()const noexcept { return m_activity; }

	Status status()const noexcept { return m_status; }
	
private:
	std::optional<::activity> m_activity;
	Status m_status = Status::unknown;

	
};