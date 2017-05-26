#include "controler.h"
#include "core.h"

namespace tke
{
	void Controller::reset()
	{
		front = back = left = right = up = down = turnLeft = turnRight = false;
		lastTime = nowTime;
	}

	bool Controller::move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler)
	{
		float dist = (nowTime - lastTime) / 1000.f;
		lastTime = nowTime;

		outCoord = glm::vec3();
		outEuler = glm::vec3();

		inEulerX = glm::radians(inEulerX + baseForwardAng);

		if (front && frontSpeed > 0.f)
		{
			outCoord.x += cos(inEulerX) * frontSpeed * dist;
			outCoord.z -= sin(inEulerX) * frontSpeed * dist;
		}
		if (back && backSpeed > 0.f)
		{
			outCoord.x -= cos(inEulerX) * backSpeed * dist;
			outCoord.z += sin(inEulerX) * backSpeed * dist;
		}
		if (left && leftSpeed > 0.f)
		{
			outCoord.x -= sin(inEulerX) * leftSpeed * dist;
			outCoord.z -= cos(inEulerX) * leftSpeed * dist;
		}
		if (right && rightSpeed > 0.f)
		{
			outCoord.x += sin(inEulerX) * rightSpeed * dist;
			outCoord.z += cos(inEulerX) * rightSpeed * dist;
		}
		if (up)
			outCoord.y += upSpeed * dist;
		if (down)
			outCoord.y -= downSpeed * dist;

		if (turnLeft)
			outEuler.x = turnLeftSpeed * dist;
		if (turnRight)
			outEuler.x = -turnRightSpeed * dist;
		if (turnUp)
			outEuler.z = turnUpSpeed * dist;
		if (turnDown)
			outEuler.z = -turnDownSpeed * dist;

		return (outCoord.x != 0.f || outCoord.y != 0.f || outCoord.z != 0.f) || (outEuler.x != 0.f) || (outEuler.y != 0.f) || (outEuler.z != 0.f);
	}

	bool Controller::keyDown(int key)
	{
		switch (key)
		{
		case 'W':
			front = true;
			return true;
		case 'S':
			back = true;
			return true;
		case 'A':
			turnLeft = true;
			return true;
		case 'D':
			turnRight = true;
			return true;
		case 'Q':
			turnUp = true;
			return true;
		case 'E':
			turnDown = true;
			return true;
		}
		return false;
	}

	bool Controller::keyUp(int key)
	{
		switch (key)
		{
		case 'W':
			front = false;
			return true;
		case 'S':
			back = false;
			return true;
		case 'A':
			turnLeft = false;
			return true;
		case 'D':
			turnRight = false;
			return true;
		case 'Q':
			turnUp = false;
			return true;
		case 'E':
			turnDown = false;
			return true;
		}
		return false;
	}
}
