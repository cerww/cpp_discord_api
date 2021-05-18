#pragma once
#include <string>
#include <iostream>


struct thingy_that_prints_after_destroyed {
	thingy_that_prints_after_destroyed() = default;

	explicit thingy_that_prints_after_destroyed(std::string s) :
		thing_to_print(std::move(s)) {};

	thingy_that_prints_after_destroyed(const thingy_that_prints_after_destroyed&) = default;
	
	thingy_that_prints_after_destroyed& operator=(const thingy_that_prints_after_destroyed&) = default;
	
	thingy_that_prints_after_destroyed(thingy_that_prints_after_destroyed&&) = delete;
	
	thingy_that_prints_after_destroyed& operator=(thingy_that_prints_after_destroyed&&) = delete;

	~thingy_that_prints_after_destroyed() {
		std::cout << thing_to_print << std::endl;
	}

	std::string thing_to_print;
};
