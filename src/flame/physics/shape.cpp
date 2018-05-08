#include "shape_private.h"
#include "rigid_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		Shape *create_box_shape(Rigid *r, Material *m, const glm::vec3 &coord,
			float x_hf_ext, float y_hf_ext, float z_hf_ext)
		{
			auto s = new Shape;

			s->_priv = new ShapePrivate;

			s->_priv->v = r->_priv->v->createShape(physx::PxBoxGeometry(x_hf_ext, y_hf_ext, z_hf_ext),
				*m->_priv->v);
			s->_priv->v->setLocalPose(Z(coord, glm::vec4(0.f, 0.f, 0.f, 1.f)));

			return s;
		}

		//void create_sphere_shape(Material *m, const glm::vec3 &coord,
		//	float radius)
		//{
		//	_priv->v->createShape(physx::PxSphereGeometry(scale[0]),
		//		*pxDefaultMaterial, trans);
		//}

		//void create_capsule_shape(Material *m, const glm::vec3 &coord,
		//	float radius, float height)
		//{
		//	_priv->v->createShape(physx::PxCapsuleGeometry(scale[0], scale[1]),
		//		*pxDefaultMaterial, trans * physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
		//}

		void destroy_shape(Shape *s)
		{
			delete s->_priv;
			delete s;
		}
	}
}

