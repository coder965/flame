#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Rigid;

		struct ScenePrivate;

		struct Scene
		{
			ScenePrivate *_priv;

			FLAME_PHYSICS_EXPORTS void add_rigid(Rigid *r);
			FLAME_PHYSICS_EXPORTS void remove_rigid(Rigid *r);
		};

		FLAME_PHYSICS_EXPORTS Scene *create_scene(Device *d, float gravity, int thread_count);
		FLAME_PHYSICS_EXPORTS void destroy_scene(Scene *s);
	}
}

