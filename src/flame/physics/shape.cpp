#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		Shape *create_Shape()
		{
			auto m = new Shape;
			
			m->_priv = new ShapePrivate;

			return m;
		}

		void destroy_shape(Shape *s)
		{
			delete s->_priv;
			delete s;
		}
	}
}

