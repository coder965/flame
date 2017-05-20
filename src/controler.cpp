#include "controler.h"

namespace tke
{
	void Controller::reset()
	{
		front = back = left = right = up = down = turnLeft = turnRight = false;
	}

	bool Controller::move(float inEulerX, glm::vec3 &outCoord, float &outEuler)
	{
		outCoord = glm::vec3();
		outEuler = 0.f;

		inEulerX = glm::radians(inEulerX + baseForwardAng);

		if (front && frontSpeed > 0.f)
		{
			outCoord.x += cos(inEulerX) * frontSpeed;
			outCoord.z -= sin(inEulerX) * frontSpeed;
		}
		if (back && backSpeed > 0.f)
		{
			outCoord.x -= cos(inEulerX) * backSpeed;
			outCoord.z += sin(inEulerX) * backSpeed;
		}
		if (left && leftSpeed > 0.f)
		{
			outCoord.x -= sin(inEulerX) * leftSpeed;
			outCoord.z -= cos(inEulerX) * leftSpeed;
		}
		if (right && rightSpeed > 0.f)
		{
			outCoord.x += sin(inEulerX) * rightSpeed;
			outCoord.z += cos(inEulerX) * rightSpeed;
		}
		if (up)
		{
			outCoord.y += upSpeed;
		}
		if (down)
		{
			outCoord.y -= downSpeed;
		}

		if (turnLeft)
			outEuler = turnSpeed[0];
		if (turnRight)
			outEuler = -turnSpeed[1];

		return (outCoord.x != 0.f || outCoord.y != 0.f || outCoord.z != 0.f) || (outEuler != 0.f);
	}
}
