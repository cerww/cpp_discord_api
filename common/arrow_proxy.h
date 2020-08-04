#pragma once

template<class Reference>
struct arrow_proxy {
	mutable Reference r;
	Reference* operator->()const {
		return &r;
	}
};
