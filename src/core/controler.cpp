#include "controler.h"

namespace tke
{
	void Controller::reset()
	{
		front = back = left = right = up = down = turnLeft = turnRight = false;
	}

	glm::vec3 Controller::move(float &eulerX)
	{
		eulerX += baseForwardAng;
		eulerX = glm::radians(eulerX);
		glm::vec3 coord;
		if (front)
		{
			coord.x += cos(eulerX) * frontSpeed;
			coord.z -= sin(eulerX) * frontSpeed;
		}
		if (back)
		{
			coord.x -= cos(eulerX) * backSpeed;
			coord.z += sin(eulerX) * backSpeed;
		}
		if (left)
		{
			coord.x -= sin(eulerX) * leftSpeed;
			coord.z -= cos(eulerX) * leftSpeed;
		}
		if (right)
		{
			coord.x += sin(eulerX) * rightSpeed;
			coord.z += cos(eulerX) * rightSpeed;
		}
		if (up)
		{
			coord.y += upSpeed;
		}
		if (down)
		{
			coord.y -= downSpeed;
		}
		eulerX = 0.f;
		if (turnLeft)
		{
			eulerX = turnSpeed[0];
		}
		if (turnRight)
		{
			eulerX = -turnSpeed[1];
		}
		return coord;
	}
}
