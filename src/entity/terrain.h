#pragma once

#include "transformer.h"
#include "node.h"

namespace physx { struct PxRigidActor; }

namespace tke
{
	struct TerrainVertex
	{
		glm::vec4 normal_height;
		glm::vec3 tangent;
	};

	struct Image;
	struct Material;

	REFLECTABLE struct Terrain : Transformer, Node
	{
		REFL_BANK;

		REFLv bool use_physx = false;

		REFLv int block_cx = 64;
		REFLv float block_size = 16.f;
		REFLv float height = 100.f;
		REFLv float tessellation_factor = 0.75f;
		REFLv float tiling_scale = 8.f;

		std::unique_ptr<TerrainVertex[]> vertex;

		std::shared_ptr<Image> blendMap;
		std::shared_ptr<Material> materials[4];

		physx::PxRigidActor *actor = nullptr;

		int sceneIndex = -1;

		Terrain(bool _use_physx = false, std::shared_ptr<Image> _blendMap = nullptr);
	};

}
