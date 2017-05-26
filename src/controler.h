#ifndef __TKE_CONTROLER__
#define __TKE_CONTROLER__

#include "math.h"

namespace tke
{
	struct Controller
	{
		int lastTime = 0;
		float baseForwardAng = 0.f;
		float frontSpeed = 1.f, backSpeed = 1.f, leftSpeed = 1.f, rightSpeed = 1.f, upSpeed = 1.f, downSpeed = 1.f;
		float turnLeftSpeed = 75.f;
		float turnRightSpeed = 75.f;
		float turnUpSpeed = 75.f;
		float turnDownSpeed = 75.f;
		bool front = false, back = false, left = false, right = false, up = false, down = false, turnLeft = false, turnRight = false, turnUp = false, turnDown = false;

		void reset();
		bool move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler);
		bool keyDown(int key);
		bool keyUp(int key);
	};
}

#endif