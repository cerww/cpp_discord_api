#pragma once

//https://quuxplusone.github.io/blog/2019/02/06/arrow-proxy/ 
template<class Reference>
struct arrow_proxy {
	mutable Reference r;
	Reference* operator->()const {
		return &r;
	}
};
