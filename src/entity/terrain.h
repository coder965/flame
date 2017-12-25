#pragma once

#include "transformer.h"
#include "node.h"

namespace physx { struct PxRigidActor; }

namespace tke
{
	struct Image;
	REFLECTABLE struct Terrain : Transformer, Node
	{
		REFL_BANK;

		REFLv bool use_physx = false;

		REFLv std::string height_map_filename;
		REFLv std::string normal_map_filename;
		REFLv std::string blend_map_filename;
		REFLv std::string color_map0_filename;
		REFLv std::string color_map1_filename;
		REFLv std::string color_map2_filename;
		REFLv std::string color_map3_filename;
		REFLv std::string normal_map0_filename;
		REFLv std::string normal_map1_filename;
		REFLv std::string normal_map2_filename;
		REFLv std::string normal_map3_filename;

		REFLv int block_cx = 64;
		REFLv float block_size = 16.f;
		REFLv float height = 100.f;
		REFLv float tessellation_factor = 0.75f;
		REFLv float texture_uv_factor = 8.f;

		Image *heightMap = nullptr;
		Image *normalMap = nullptr;
		Image *blendMap = nullptr;
		Image *colorMaps[4] = {};
		Image *normalMaps[4] = {};
		float spec = 0.04f;
		float roughness = 1.f;

		physx::PxRigidActor *actor = nullptr;

		Terrain(bool _use_physx = false, Image *_heightMap = nullptr, Image *_normalMap = nullptr, Image *_blendMap = nullptr,
			Image *_colorMap0 = nullptr, Image *_colorMap1 = nullptr, Image *_colorMap2 = nullptr, Image *_colorMap3 = nullptr,
			Image *_normalMap0 = nullptr, Image *_normalMap1 = nullptr, Image *_normalMap2 = nullptr, Image *_normalMap3 = nullptr);
	};

}
