#include "device.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate
		{
			physx::PxFoundation *foundation;
			physx::PxPhysics *inst;
		};
	}
}

