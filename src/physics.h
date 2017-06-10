#ifndef __TKE_PHYSICS__
#define __TKE_PHYSICS__

#include <PxPhysicsAPI.h>

namespace tke
{
	extern physx::PxFoundation *pxFoundation;
	extern physx::PxPhysics *pxPhysics;
	extern physx::PxMaterial *pxDefaultMaterial;

	physx::PxRigidActor *createStaticRigidActor(physx::PxTransform &trans);
	physx::PxRigidActor *createDynamicRigidActor(physx::PxTransform &trans, bool kinematic, float density);
}

#endif