#pragma once

#include "device.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate
		{
			physx::PxDefaultAllocator allocator;
			physx::PxDefaultErrorCallback error_callback;
			physx::PxFoundation *foundation;
			physx::PxPhysics *inst;
		};
	}
}

