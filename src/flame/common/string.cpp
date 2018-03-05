#define NOMINMAX
#include <Windows.h>

#include <flame/common/string.h>

namespace flame
{
	int get_str_line_number(const char *str)
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

	std::string string_cut(const std::string &str, int length)
	{
		if (length < 0)
			length = str.size() + length;
		return std::string(str.begin(), str.begin() + length);
	}

	bool string_contain(const std::string &str, char v)
	{
		for (auto &c : str)
		{
			if (c == v)
				return true;
		}
		return false;
	}

	std::string translate(int srcCP, int dstCP, const std::string &src)
	{
		auto wbuf = new wchar_t[src.size() + 1];
		MultiByteToWideChar(srcCP, 0, src.c_str(), -1, wbuf, src.size() + 1);
		auto buf = new char[src.size() + 1];
		WideCharToMultiByte(dstCP, 0, wbuf, -1, buf, src.size() + 1, NULL, false);
		delete[]wbuf;
		std::string str(buf);
		delete[]buf;
		return str;
	}

	std::string japanese_to_chinese(const std::string &src)
	{
		return translate(932, 936, src);
	}
}
