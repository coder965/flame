#pragma once

#include <string>

namespace flame
{
	inline std::string to_str(bool v)
	{
		return v ? "true" : "false";
	}

	inline std::string to_str(int v)
	{
		return std::to_string(v);
	}

	inline std::string to_str(float v)
	{
		auto str = std::to_string(v);
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);
		return str;
	}

	inline bool to_bool(const std::string &v)
	{
		return v == "true";
	}

	inline int to_int(const std::string &v)
	{
		return std::stoi(v);
	}

	inline float to_float(const std::string &v)
	{
		return std::stof(v);
	}
}
