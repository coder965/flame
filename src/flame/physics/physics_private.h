#pragma once

#pragma once

#include "physics.h"

#include <PxPhysicsAPI.h>
using namespace physx;

namespace flame
{
	namespace physics
	{
		inline PxVec3 Z(const glm::vec3 &v)
		{
			return PxVec3(v.x, v.y, v.z);
		}

		inline glm::vec3 Z(const PxVec3 &v)
		{
			return glm::vec3(v.x, v.y, v.z);
		}

		inline PxMat33 Z(const glm::mat3 &m)
		{
			return PxMat33(
				PxVec3(m[0][0], m[0][1], m[0][2]),
				PxVec3(m[1][0], m[1][1], m[1][2]),
				PxVec3(m[2][0], m[2][1], m[2][2])
			);
		}

		inline PxTransform Z(const glm::vec3 &coord, const glm::vec4 &quat)
		{
			return PxTransform(Z(coord), PxQuat(quat.x, quat.y, quat.z, quat.w));
		}

		inline PxTransform Z(const glm::vec3 &coord, const glm::mat3 &axis)
		{
			return PxTransform(Z(coord), PxQuat(Z(axis)));
		}
	}
}

