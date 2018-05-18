//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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
