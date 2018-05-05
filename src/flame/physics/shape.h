#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct ShapePrivate;

		struct Shape
		{
			ShapePrivate *_priv;
		};

		FLAME_PHYSICS_EXPORTS Shape *create_shape();
		FLAME_PHYSICS_EXPORTS void destroy_shape(Shape *s);
	}
}

