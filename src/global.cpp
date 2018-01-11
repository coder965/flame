#include "global.h"

namespace tke
{
	bool only_2d = false;

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
