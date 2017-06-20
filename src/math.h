#pragma once

#include <fstream>

#include "../../../glm/glm.hpp"
#include "../../../glm/gtc/matrix_transform.hpp"
#include "../../../glm/gtx/transform2.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

bool inline operator < (glm::vec3 &a, glm::vec3 &b)
{
	return a.x < b.x && a.y < b.y && a.z < b.z;
}
bool inline operator > (glm::vec3 &a, glm::vec3 &b)
{
	return a.x > b.x && a.y > b.y && a.z > b.z;
}
bool inline operator <= (glm::vec3 &a, glm::vec3 &b)
{
	return a.x <= b.x && a.y <= b.y && a.z <= b.z;
}
bool inline operator >= (glm::vec3 &a, glm::vec3 &b)
{
	return a.x >= b.x && a.y >= b.y && a.z >= b.z;
}

bool inline operator < (float a, glm::vec3 &b)
{
	return a < b.x && a < b.y && a < b.z;
}
bool inline operator > (float a, glm::vec3 &b)
{
	return a > b.x && a > b.y && a > b.z;
}
bool inline operator <= (float a, glm::vec3 &b)
{
	return a <= b.x && a <= b.y && a <= b.z;
}
bool inline operator >= (float a, glm::vec3 &b)
{
	return a >= b.x && a >= b.y && a >= b.z;
}

bool inline operator < (glm::vec3 a, float b)
{
	return a.x < b && a.y < b && a.z < b;
}
bool inline operator > (glm::vec3 a, float b)
{
	return a.x > b && a.y > b && a.z > b;
}
bool inline operator <= (glm::vec3 a, float b)
{
	return a.x <= b && a.y <= b && a.z <= b;
}
bool inline operator >= (glm::vec3 a, float b)
{
	return a.x >= b && a.y >= b && a.z >= b;
}

namespace tke
{
	void quaternionToMatrix(glm::vec4 &q, glm::mat3 &mat);
	void matrixToQuaternion(glm::mat3 &mat, glm::vec4 &q);
	void quaternionRotate(glm::vec4 &q, glm::vec3 v);
	void eulerYzxToMatrix(glm::vec3 &e, glm::mat3 &mat);
	void eulerYxzToMatrix(glm::vec3 &e, glm::mat3 &mat);
	glm::mat4 makeMatrix(glm::mat3 &rotation, glm::vec3 coord);

	float rand2d(glm::vec2 v);
	float noise2d(glm::vec2 v);
	float fbm2d(glm::vec2 v);
}
