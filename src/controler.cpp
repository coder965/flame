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

		if (front && speed > 0.f)
		{
			outCoord.x -= sin(inEulerX) * speed * dist;
			outCoord.z -= cos(inEulerX) * speed * dist;
		}
		if (back && speed > 0.f)
		{
			outCoord.x += sin(inEulerX) * speed * dist;
			outCoord.z += cos(inEulerX) * speed * dist;
		}
		if (left && speed > 0.f)
		{
			outCoord.x -= cos(inEulerX) * speed * dist;
			outCoord.z += sin(inEulerX) * speed * dist;
		}
		if (right && speed > 0.f)
		{
			outCoord.x += cos(inEulerX) * speed * dist;
			outCoord.z -= sin(inEulerX) * speed * dist;
		}
		if (up)
			outCoord.y += speed * dist;
		if (down)
			outCoord.y -= speed * dist;

		if (turnLeft)
			outEuler.x = turnSpeed * dist;
		if (turnRight)
			outEuler.x = -turnSpeed * dist;
		if (turnUp)
			outEuler.z = turnSpeed * dist;
		if (turnDown)
			outEuler.z = -turnSpeed * dist;

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
			left = true;
			return true;
		case 'D':
			right = true;
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
			left = false;
			return true;
		case 'D':
			right = false;
			return true;
		}
		return false;
	}
}
