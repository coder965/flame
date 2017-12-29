#include "string_utils.h"

namespace tke
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
}
