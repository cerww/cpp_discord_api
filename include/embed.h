#pragma once
#include <nlohmann/json.hpp>

struct embed { };
<<<<<<< HEAD

inline void from_json(const nlohmann::json&, embed&) { }

inline void to_json(nlohmann::json&,const embed&) {
	
}
=======

inline void from_json(const nlohmann::json&, embed&) { };
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
