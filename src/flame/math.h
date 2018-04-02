#pragma once

#include <flame/exports.h>

#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

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

#define TK_EPS (0.000001f)

namespace flame
{
	inline int digit_num(int a)
	{
		auto d = 0;
		do
		{
			d++;
			a /= 10;
		} while (a);
		return d;
	}

	inline bool is_same(const float a, const float b)
	{
		return std::abs(a - b) <= TK_EPS;
	}

	inline bool is_same(const glm::vec2 &a, const glm::vec2 &b)
	{
		return 
			std::abs(a.x - b.x) <= TK_EPS &&
			std::abs(a.y - b.y) <= TK_EPS;
	}

	inline bool is_same(const glm::vec3 &a, const glm::vec3 &b)
	{
		return
			std::abs(a.x - b.x) <= TK_EPS &&
			std::abs(a.y - b.y) <= TK_EPS &&
			std::abs(a.z - b.z) <= TK_EPS;
	}

	inline bool is_same(const glm::vec4 &a, const glm::vec4 &b)
	{
		return
			std::abs(a.x - b.x) <= TK_EPS &&
			std::abs(a.y - b.y) <= TK_EPS &&
			std::abs(a.z - b.z) <= TK_EPS &&
			std::abs(a.w - b.w) <= TK_EPS;
	}

	inline glm::vec3 transform(const glm::vec3 &v, const glm::mat4 &mat)
	{
		auto v_ = mat * glm::vec4(v, 1.f);
		v_ /= v_.w;
		return glm::vec3(v_);
	}

	inline glm::vec4 plane(const glm::vec3 &p, const glm::vec3 &normal)
	{
		return glm::vec4(normal, glm::dot(normal, p));
	}

	inline void ortho_normalize(glm::mat3 &mat)
	{
		for (auto i = 0; i < 3; i++)
			mat[i] = glm::normalize(mat[i]);
	}

	inline void ortho_normalize(glm::mat4 &mat)
	{
		for (auto i = 0; i < 3; i++)
			mat[i] = glm::vec4(glm::normalize(glm::vec3(mat[i])), 0.f);
	}

	FLAME_EXPORTS float ray_intersect_plane(const glm::vec3 &origin, const glm::vec3 &vector, const glm::vec4 &plane);

	FLAME_EXPORTS glm::mat3 quaternion_to_mat3(glm::vec4 &q);

	FLAME_EXPORTS glm::vec4 mat3_to_quaternion(glm::mat3 &mat);

	FLAME_EXPORTS glm::vec3 quaternion_to_euler(glm::vec4 &q);

	FLAME_EXPORTS void quaternion_rotate(glm::vec4 &q, glm::vec3 &v);

	FLAME_EXPORTS glm::mat3 euler_to_mat3(glm::vec3 &e);

	inline glm::mat3 euler_to_mat3(float x, float y, float z)
	{
		return euler_to_mat3(glm::vec3(x, y, z));
	}

	inline glm::mat4 make_matrix(const glm::mat3 &rotation, const glm::vec3 coord)
	{
		return glm::mat4(
			glm::vec4(rotation[0], 0.f),
			glm::vec4(rotation[1], 0.f),
			glm::vec4(rotation[2], 0.f),
			glm::vec4(coord, 1.f)
		);
	}

	inline glm::mat4 make_matrix(const glm::vec3 &x, const glm::vec3 &y, const glm::vec3 coord)
	{
		return glm::mat4(
			glm::vec4(x, 0.f),
			glm::vec4(y, 0.f),
			glm::vec4(glm::cross(x, y), 0.f),
			glm::vec4(coord, 1.f)
		);
	}

	inline float linear_depth_ortho(float z, float depth_near, float depth_far)
	{
		z = z * 0.5 + 0.5;
		return z * (depth_far - depth_near) + depth_near;
	}

	inline float linear_depth_perspective(float z, float depth_near, float depth_far)
	{
		float a = (1.0 - depth_far / depth_near) * 0.5 / depth_far;
		float b = (1.0 + depth_far / depth_near) * 0.5 / depth_far;
		return 1.0 / (a * z + b);
	}

	FLAME_EXPORTS float rand2d(const glm::vec2 &v);
	FLAME_EXPORTS float noise2d(glm::vec2 v);
	FLAME_EXPORTS float fbm2d(glm::vec2 v);

	FLAME_EXPORTS glm::vec4 fit_rect(const glm::vec2 &desired_size, float xy_aspect);
	FLAME_EXPORTS glm::vec4 fit_rect_no_zoom_in(const glm::vec2 &desired_size, const glm::vec2 &size);
}
