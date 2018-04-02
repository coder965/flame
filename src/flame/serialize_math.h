#pragma once

#include <string>

#include "serialize.h"
#include "math.h"

namespace flame
{
	inline std::string to_str(const glm::ivec2 &v)
	{
		return to_str(v.x) + "/" + to_str(v.y);
	}

	inline std::string to_str(const glm::ivec3 &v)
	{
		return to_str(v.x) + "/" + to_str(v.y) + "/" + to_str(v.z);
	}

	inline std::string to_str(const glm::ivec4 &v)
	{
		return to_str(v.x) + "/" + to_str(v.y) + "/" + to_str(v.z) + "/" + to_str(v.w);
	}

	inline std::string to_str(const glm::vec2 &v)
	{
		return to_str(v.x) + "/" + to_str(v.y);
	}

	inline std::string to_str(const glm::vec3 &v)
	{
		return to_str(v.x) + "/" + to_str(v.y) + "/" + to_str(v.z);
	}

	inline std::string to_str(const glm::vec4 &v)
	{
		return to_str(v.x) + "/" + to_str(v.y) + "/" + to_str(v.z) + "/" + to_str(v.w);
	}

	inline glm::ivec2 to_int2(const std::string &v)
	{
		glm::ivec2 x;
		assert(sscanf(v.c_str(), "%d/%d", &x.x, &x.y) == 2);
		return x;
	}

	inline glm::ivec3 to_int3(const std::string &v)
	{
		glm::ivec3 x;
		assert(sscanf(v.c_str(), "%d/%d/%d", &x.x, &x.y, &x.z) == 3);
		return x;
	}

	inline glm::ivec4 to_int4(const std::string &v)
	{
		glm::ivec4 x;
		assert(sscanf(v.c_str(), "%d/%d/%d/%d", &x.x, &x.y, &x.z, &x.w) == 4);
		return x;
	}

	inline glm::vec2 to_float2(const std::string &v)
	{
		glm::vec2 x;
		assert(sscanf(v.c_str(), "%f/%f", &x.x, &x.y) == 2);
		return x;
	}

	inline glm::vec3 to_float3(const std::string &v)
	{
		glm::vec3 x;
		assert(sscanf(v.c_str(), "%f/%f/%f", &x.x, &x.y, &x.z) == 3);
		return x;
	}

	inline glm::vec4 to_float4(const std::string &v)
	{
		glm::vec4 x;
		assert(sscanf(v.c_str(), "%f/%f/%f/%f", &x.x, &x.y, &x.z, &x.w) == 4);
		return x;
	}
}
