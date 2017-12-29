#pragma once

#include <string>

namespace tke
{
	enum Err
	{
		NoErr,
		ErrInvalidEnum,
		ErrInvalidValue,
		ErrInvalidOperation,
		ErrOutOfMemory,
		ErrContextLost,
		ErrResourceLost
	};

	std::string get_error_string(Err errNum);
}
