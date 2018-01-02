#pragma once

#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "../../glm/glm/glm.hpp"
#include "../../glm/glm/gtc/matrix_transform.hpp"
#include "../../glm/glm/gtx/transform2.hpp"

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

#define TK_EPS (0.001f)

namespace tke
{
	inline bool fEqual(const float a, const float b)
	{
		return std::abs(a - b) <= TK_EPS;
	}

	inline bool fEqual(const glm::vec2 &a, const glm::vec2 &b)
	{
		return 
			std::abs(a.x - b.x) <= TK_EPS &&
			std::abs(a.y - b.y) <= TK_EPS;
	}

	inline bool fEqual(const glm::vec3 &a, const glm::vec3 &b)
	{
		return
			std::abs(a.x - b.x) <= TK_EPS &&
			std::abs(a.y - b.y) <= TK_EPS &&
			std::abs(a.z - b.z) <= TK_EPS;
	}

	inline bool fEqual(const glm::vec4 &a, const glm::vec4 &b)
	{
		return
			std::abs(a.x - b.x) <= TK_EPS &&
			std::abs(a.y - b.y) <= TK_EPS &&
			std::abs(a.z - b.z) <= TK_EPS &&
			std::abs(a.w - b.w) <= TK_EPS;
	}

	glm::mat3 quaternion_to_mat3(glm::vec4 &q);
	glm::vec4 mat3_to_quaternion(glm::mat3 &mat);
	void quaternion_rotate(glm::vec4 &q, glm::vec3 &v);
	glm::mat3 euler_yzx_to_mat3(glm::vec3 &e);
	glm::mat3 euler_yxz_to_mat3(glm::vec3 &e);
	glm::mat4 makeMatrix(glm::mat3 &rotation, glm::vec3 coord);

	float linearDepthOrtho(float z, float depth_near, float depth_far);
	float linearDepthPerspective(float z, float depth_near, float depth_far);
	float rand2d(glm::vec2 v);
	float noise2d(glm::vec2 v);
	float fbm2d(glm::vec2 v);
}
