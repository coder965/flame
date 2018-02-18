#include <flame/global.h>

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

	long long now_ns;
	double elapsed_time;

	std::string engine_path;

	unsigned long long total_frame_count = 0;
	uint32_t FPS;
}