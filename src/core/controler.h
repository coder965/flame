#ifndef __TKE_CONTROLER__
#define __TKE_CONTROLER__

#include "math.h"

namespace tke
{
	struct Controller
	{
		void reset();
		glm::vec3 move(float &eulerX);

		float baseForwardAng = 0.f;
		float frontSpeed = 0.1f, backSpeed = 0.1f, leftSpeed = 0.1f, rightSpeed = 0.1f, upSpeed = 0.1f, downSpeed = 0.1f;
		glm::vec2 turnSpeed = glm::vec2(1.f); // 0 - left, 1 - right
		bool front = false, back = false, left = false, right = false, up = false, down = false, turnLeft = false, turnRight = false;
	};
}

#endif