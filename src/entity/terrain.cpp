#include "../global.h"
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
		enable_physics(_use_physx), blend_image(_blendMap)
	{


		if (!blend_image)
			 blend_image = default_blend_image;

		add_material(defaultMaterial);
	}

	void Terrain::add_material(std::shared_ptr<Material> m)
	{
		if (material_count == TK_ARRAYSIZE(materials))
			return;
		materials[material_count] = m;
		material_count++;
		attribute_dirty = true;
	}

	void Terrain::remove_material(int index)
	{
		if (index >= material_count)
			return;
		for (int i = index; i < material_count - 1; i++)
			materials[i] = materials[i + 1];
		material_count--;
		materials[material_count].reset();
		attribute_dirty = true;
	}
}
