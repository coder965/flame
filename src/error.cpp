#include "error.h"

namespace tke
{
	std::string get_error_string(Err errNum)
	{
		switch (errNum)
		{
		case NoErr:
			return "No error.";
		case ErrInvalidEnum:
			return "Invalid enum.";
		case ErrInvalidValue:
			return "Invalid value.";
		case ErrInvalidOperation:
			return "Invalid operation.";
		case ErrOutOfMemory:
			return "Out of memory.";
		case ErrContextLost:
			return "Context lost.";
		case ErrResourceLost:
			return "Resource lost.";
		default:
			return "unknow error";
		}
	}
}
