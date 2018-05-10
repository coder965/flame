#pragma once

#ifdef _FLAME_PHYSICS_EXPORTS
#define FLAME_PHYSICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_PHYSICS_EXPORTS __declspec(dllimport)
#endif

#include <flame/math.h>

namespace flame
{
	namespace physics
	{
		enum TouchType
		{
			TouchFound,
			TouchLost
		};
	}
}

