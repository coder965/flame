#include "rigid_private.h"

namespace flame
{
	namespace physics
	{
		Rigid *create_static_rigid()
		{
			auto r = new Rigid;
			
			r->_priv = new RigidPrivate;

			return r;
		}

		Rigid *create_dynamic_rigid()
		{
			auto r = new Rigid;

			r->_priv = new RigidPrivate;

			return r;
		}

		void destroy_material(Rigid *r)
		{
			delete r->_priv;
			delete r;
		}
	}
}

