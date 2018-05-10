#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		void Rigid::attach_shape(Shape *s)
		{
			_priv->v->attachShape(*s->_priv->v);
		}

		void Rigid::detach_shape(Shape *s)
		{
			_priv->v->detachShape(*s->_priv->v);
		}

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

		void Rigid::add_force(const glm::vec3 &v)
		{
			auto d = (PxRigidDynamic*)_priv->v;
			d->addForce(Z(v));
		}

		void Rigid::clear_force()
		{
			auto d = (PxRigidDynamic*)_priv->v;
			d->clearForce();
		}
		
		Rigid *create_static_rigid(Device *d, const glm::vec3 &coord)
		{
			auto r = new Rigid;
			
			r->_priv = new RigidPrivate;
			r->_priv->v = d->_priv->inst->createRigidStatic(
				Z(coord, glm::vec4(0.f, 0.f, 0.f, 1.f)));
			r->_priv->v->userData = r;

			return r;
		}

		Rigid *create_dynamic_rigid(Device *d, const glm::vec3 &coord)
		{
			auto r = new Rigid;

			r->_priv = new RigidPrivate;
			r->_priv->v = d->_priv->inst->createRigidDynamic(
				Z(coord, glm::vec4(0.f, 0.f, 0.f, 1.f)));
			//PxRigidBodyExt::updateMassAndInertia(*body, density);
			//if (kinematic) body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			r->_priv->v->userData = r;

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

