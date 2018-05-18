//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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
