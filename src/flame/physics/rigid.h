#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct RigidPrivate;

		struct Rigid
		{
			RigidPrivate *_priv;
		};

		FLAME_PHYSICS_EXPORTS Rigid *create_static_rigid();
		FLAME_PHYSICS_EXPORTS Rigid *create_dynamic_rigid();
		FLAME_PHYSICS_EXPORTS void destroy_rigid(Rigid *r);
	}
}

