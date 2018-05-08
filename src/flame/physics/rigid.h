#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Material;

		struct RigidPrivate;

		struct Rigid
		{
			RigidPrivate *_priv;

			FLAME_PHYSICS_EXPORTS void get_pose(glm::vec3 &out_coord, glm::vec4 &out_quat);
		};

		FLAME_PHYSICS_EXPORTS Rigid *create_static_rigid(Device *d, const glm::vec3 &coord);
		FLAME_PHYSICS_EXPORTS Rigid *create_dynamic_rigid(Device *d, const glm::vec3 &coord);
		FLAME_PHYSICS_EXPORTS void destroy_rigid(Rigid *r);
	}
}

