#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/material.h"
#include "../graphics/pipeline.h"
#include "terrain.h"

namespace tke
{
	Terrain::Terrain(int _block_cx, int _block_cy, bool _use_physx, 
		std::shared_ptr<Image> _blendMap)
		:Node(NodeTypeTerrain), block_cx(_block_cx), block_cy(_block_cy), 
		use_physx(_use_physx), blendMap(_blendMap)
	{


		if (!blendMap)
			 blendMap = default_blend_image;

		add_material(defaultMaterial);
	}

	void Terrain::add_material(std::shared_ptr<Material> m)
	{
		if (material_count == TK_ARRAYSIZE(materials))
			return;
		materials[material_count] = m;
		material_count++;
	}

	void Terrain::remove_material(int index)
	{
		if (index >= material_count)
			return;
		for (int i = index; i < material_count - 1; i++)
			materials[i] = materials[i + 1];
		material_count--;
		materials[material_count].reset();
	}

	void init_terrain()
	{
		{
			static VkVertexInputBindingDescription bindings = { 0, sizeof(TerrainVertex), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes[] = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(TerrainVertex, normal_height) },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT,	offsetof(TerrainVertex, tangent) }
			};

			terrianVertexInputState = vertexStateInfo(1, &bindings, TK_ARRAYSIZE(attributes), attributes);
		}
	}
}
