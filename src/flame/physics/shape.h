#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Rigid;
		struct Material;

		struct ShapePrivate;

		struct Shape
		{
			ShapePrivate *_priv;
		};

		//FLAME_PHYSICS_EXPORTS void create_box_shape(Material *m, const glm::vec3 &coord,
		//	float x_hf_ext, float y_hf_ext, float z_hf_ext);
		//FLAME_PHYSICS_EXPORTS void create_sphere_shape(Material *m, const glm::vec3 &coord,
		//	float radius);
		//FLAME_PHYSICS_EXPORTS void create_capsule_shape(Material *m, const glm::vec3 &coord,
		//	float radius, float height);

		FLAME_PHYSICS_EXPORTS Shape *create_box_shape(Rigid *r, Material *m, const glm::vec3 &coord,
			float x_hf_ext, float y_hf_ext, float z_hf_ext);
		FLAME_PHYSICS_EXPORTS void remove_shape(Shape *s, Rigid *r);
		FLAME_PHYSICS_EXPORTS void destroy_shape(Shape *s);
	}
}

