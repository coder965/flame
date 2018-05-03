#pragma once

#include <flame/global.h>

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

	struct LongString : BasicString<1024> // for output, article, description, etc
	{
	};

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
