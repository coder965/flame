#pragma once

#include "math/math.h"

namespace tke
{
	struct Vertex2D
	{
		glm::vec2       pos;
		glm::vec2       uv;
		std::uint32_t   col;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct VertexAnimated
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;

		glm::vec4 boneWeight;
		glm::vec4 boneID;
	};

	struct VertexLine
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	extern int lastTime;
	extern int nowTime;
	extern int timeDisp;

	extern std::string enginePath;

	extern int resCx;
	extern int resCy;

	extern float screenAspect;

	extern glm::mat4 matOrtho;
	extern glm::mat4 matOrthoInv;
	extern glm::mat4 matPerspective;
	extern glm::mat4 matPerspectiveInv;
}
