#ifndef __TKE_CONTROLER__
#define __TKE_CONTROLER__

#include "math.h"

namespace tke
{
	struct Controller
	{
		int lastTime = 0;
		float baseForwardAng = 0.f;
		float speed = 1.f;
		float turnSpeed = 75.f;
		bool front = false, back = false, left = false, right = false, up = false, down = false, turnLeft = false, turnRight = false, turnUp = false, turnDown = false;

		void reset();
		bool move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler);
		bool keyDown(int key);
		bool keyUp(int key);
	};
}

#endif