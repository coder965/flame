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

#include <flame/type.h>

#include <string>
#include <locale>

inline constexpr unsigned int _HASH(char const * str, unsigned int seed)
{
	return 0 == *str ? seed : _HASH(str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

#define HASH(x) (_HASH(x, 0))

template <unsigned int N>
struct EnsureConst
{
	static const unsigned int value = N;
};

#define CHASH(x) (EnsureConst<_HASH(x, 0)>::value)

namespace flame
{
	template<uint size>
	struct BasicString
	{
		char data[size];

		const char* find(char c)
		{
			auto p = data;
			while (*p && *p != c)
				p++;
			return p;
		}
	};

	struct ShortString : BasicString<32> // for name, title, number etc
	{
	};

	struct MediumString : BasicString<256> // for tip, filepath/filename, etc
	{
	};

	struct LongString : BasicString<1024 * 8> // for output, article, description, etc
	{
	};

	inline bool is_slash(char ch)
	{
		return ch == '\\' || ch == '/';
	}

	inline int get_str_line_number(const char *str)
	{
		int lineNumber = 0;
		while (*str)
		{
			if (*str == '\n')
				lineNumber++;
			str++;
		}
		return lineNumber;
	}

	inline std::string string_cut(const std::string &str, int length)
	{
		if (length < 0)
			length = str.size() + length;
		return std::string(str.begin(), str.begin() + length);
	}

	inline std::string translate(const char *src_locale, const char *dst_locale, const std::string &src)
	{
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> 
			cv1(new std::codecvt_byname<wchar_t, char, mbstate_t>(src_locale));
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> 
			cv2(new std::codecvt_byname<wchar_t, char, mbstate_t>(dst_locale));
		return cv2.to_bytes(cv1.from_bytes(src));
	}

	inline std::string japanese_to_chinese(const std::string &src)
	{
		return translate(".932", ".936", src);
	}
}
