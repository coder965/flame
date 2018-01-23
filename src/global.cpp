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

	int Resolution::x() const
	{
		return _x;
	}

	int Resolution::y() const
	{
		return _y;
	}

	float Resolution::aspect() const
	{
		return _aspect;
	}

	long long Resolution::dirty_frame() const
	{
		return _dirty_frame;
	}

	void Resolution::set(int x, int y)
	{
		_x = x;
		_y = y;
		_aspect = (float)_x / _y;

		_dirty_frame = total_frame_count;
	}

	void Resolution::set_x(int x)
	{
		_x = x;
		_aspect = (float)_x / _y;

		_dirty_frame = total_frame_count;
	}

	void Resolution::set_y(int y)
	{
		_y = y;
		_aspect = (float)_x / _y;

		_dirty_frame = total_frame_count;
	}

	Resolution resolution;

	unsigned long long total_frame_count = 0;
	uint32_t FPS;
}
