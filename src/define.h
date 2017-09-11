#pragma once

#include "math/math.h"

#define TKE_NEAR (0.1f)
#define TKE_FAR (1000.f)
#define TKE_FOVY (60.f)
#define TKE_MAX_STATIC_OBJECT_COUNT (1024)
#define TKE_MAX_ANIMATED_OBJECT_COUNT (8)
#define TKE_MAX_BONE_COUNT (256)
#define TKE_MAX_LIGHT_COUNT (256)
#define TKE_MAX_MATERIAL_COUNT (256)
#define TKE_MAX_TEXTURE_COUNT (256)
#define TKE_MAX_INDIRECT_COUNT (1024)
#define TKE_ENVR_SIZE_CX (128 * 4)
#define TKE_ENVR_SIZE_CY (128 * 2)
#define TKE_SHADOWMAP_CX (2048)
#define TKE_SHADOWMAP_CY (2048)
#define TKE_MAX_SHADOW_COUNT (8)
#define TKE_MAX_WATER_COUNT (8)

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

	extern float aspect;

	extern glm::mat4 matOrtho;
	extern glm::mat4 matOrthoInv;
	extern glm::mat4 matPerspective;
	extern glm::mat4 matPerspectiveInv;
}
