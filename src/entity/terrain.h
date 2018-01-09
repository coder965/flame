#pragma once

#include "../graphics/graphics.h"
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

	IMPL() VkPipelineVertexInputStateCreateInfo terrianVertexInputState;

	struct Image;
	struct Material;
	struct VertexBuffer;

	REFLECTABLE struct Terrain : Transformer, Node
	{
		REFL_BANK;

		REFLv bool use_physx = false;

		REFLv int block_cx;
		REFLv int block_cy;
		REFLv float block_size = 16.f;
		REFLv float height = 100.f;
		REFLv float displacement_height = 1.f;
		REFLv float tessellation_factor = 0.75f;
		REFLv float tiling_scale = 8.f;

		std::unique_ptr<TerrainVertex[]> vertex;
		std::unique_ptr<VertexBuffer> vertex_buffer;

		std::shared_ptr<Image> blendMap;
		int material_count = 0;
		std::shared_ptr<Material> materials[4];

		physx::PxRigidActor *actor = nullptr;

		Terrain(int _block_cx = 64, int _block_cy = 64, bool _use_physx = false, 
			std::shared_ptr<Image> _blendMap = nullptr);
		void add_material(std::shared_ptr<Material> m);
		void remove_material(int index);
	};

	void init_terrain();
}
