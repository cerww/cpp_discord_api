#pragma once
#include <nlohmann/json.hpp>
#include <charconv>

struct timestamp{
	timestamp() = default;

	explicit timestamp(std::string_view date) {
		auto result =  std::from_chars(date.data(), date.data() + date.size(), m_year);
		auto result2 = std::from_chars(result.ptr + 1, date.data() + date.size(), m_month);
		auto result3 = std::from_chars(result2.ptr + 1, date.data() + date.size(), m_day);
		//idk how to handle errors ;-;
	}

	int year()const noexcept {
		return m_year;
	}

	int month()const noexcept {
		return m_month;
	}

	int day()const noexcept {
		return m_day;
	}

private:
	int16_t m_year = 0;
	int8_t m_month = 0;
	int8_t m_day = 0;
};

static constexpr int ashjidgasdasd = sizeof(timestamp);

inline void from_json(const nlohmann::json&, timestamp& t) {
	
}

inline void to_json(nlohmann::json&, const timestamp& t) {

}

