#pragma once

#include "scene.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ScenePrivate
		{
			physx::PxScene *v;
		};
	}
}

