#ifndef __TKE_PHYSICS__
#define __TKE_PHYSICS__

#include <PxPhysicsAPI.h>

namespace tke
{
	void initPhysics();
	void createPhysicsScene();
	void destoryPhysicsScene();
	void syncPhysics();
}

#endif