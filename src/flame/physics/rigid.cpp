#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"

namespace flame
{
	namespace physics
	{
		void Rigid::get_pose(glm::vec3 &out_coord, glm::vec4 &out_quat)
		{
			auto trans = _priv->v->getGlobalPose();
			out_coord.x = trans.p.x;
			out_coord.y = trans.p.y;
			out_coord.z = trans.p.z;
			out_quat.x = trans.q.x;
			out_quat.y = trans.q.y;
			out_quat.z = trans.q.z;
			out_quat.w = trans.q.w;
		}
		
		Rigid *create_static_rigid(Device *d, const glm::vec3 &coord)
		{
			auto r = new Rigid;
			
			r->_priv = new RigidPrivate;
			r->_priv->v = d->_priv->inst->createRigidStatic(
				Z(coord, glm::vec4(0.f, 0.f, 0.f, 1.f)));

			return r;
		}

		Rigid *create_dynamic_rigid(Device *d, const glm::vec3 &coord)
		{
			auto r = new Rigid;

			r->_priv = new RigidPrivate;
			r->_priv->v = d->_priv->inst->createRigidDynamic(
				Z(coord, glm::vec4(0.f, 0.f, 0.f, 1.f)));
			//physx::PxRigidBodyExt::updateMassAndInertia(*body, density);
			//if (kinematic) body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

			return r;
		}

		void destroy_rigid(Rigid *r)
		{
			r->_priv->v->release();
			delete r->_priv;
			delete r;
		}
	}
}

