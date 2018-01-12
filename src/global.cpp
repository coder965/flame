#include "global.h"

namespace tke
{
	bool only_2d = false;

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

	float near_plane = 0.1f;
	float far_plane = 1000.f;
	float fovy = 60.f;

	int nowTime = 0;

	std::string engine_path;
	int res_cx;
	int res_cy;
	float res_aspect;

	uint32_t FPS;
}
