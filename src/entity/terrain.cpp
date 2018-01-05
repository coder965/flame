#include "../graphics/image.h"
#include "../graphics/material.h"
#include "terrain.h"

namespace tke
{
	Terrain::Terrain(bool _use_physx, std::shared_ptr<Image> _blendMap)
		:Node(NodeTypeTerrain), use_physx(_use_physx), blendMap(_blendMap)
	{
		if (!blendMap)
			 blendMap = default_blend_image;

		for (int i = 0; i < 4; i++)
			materials[i] = defaultMaterial;
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
}
