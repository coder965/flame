#pragma once

#pragma once

#include "physics.h"

#include <PxPhysicsAPI.h>

namespace flame
{
	namespace physics
	{
		inline physx::PxVec3 Z(const glm::vec3 &v)
		{
			return physx::PxVec3(v.x, v.y, v.z);
		}

		inline glm::vec3 Z(const physx::PxVec3 &v)
		{
			return glm::vec3(v.x, v.y, v.z);
		}

		inline physx::PxMat33 Z(const glm::mat3 &m)
		{
			return physx::PxMat33(
				physx::PxVec3(m[0][0], m[0][1], m[0][2]),
				physx::PxVec3(m[1][0], m[1][1], m[1][2]),
				physx::PxVec3(m[2][0], m[2][1], m[2][2])
			);
		}

		inline physx::PxTransform Z(const glm::vec3 &coord, const glm::vec4 &quat)
		{
			return physx::PxTransform(Z(coord), physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
		}

		inline physx::PxTransform Z(const glm::vec3 &coord, const glm::mat3 &axis)
		{
			return physx::PxTransform(Z(coord), physx::PxQuat(Z(axis)));
		}
	}
}

