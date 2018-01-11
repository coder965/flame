#pragma once

#include "../graphics/graphics.h"
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
	struct VertexBuffer;

	struct Terrain : Node
	{
		bool enable_physics = false;

		int block_cx;
		int block_cy;
		float block_size = 16.f;
		float height = 100.f;
		float displacement_height = 1.f;
		float tessellation_factor = 0.75f;
		float tiling_scale = 8.f;

		std::unique_ptr<TerrainVertex[]> vertex;
		std::unique_ptr<VertexBuffer> vertex_buffer;

		std::shared_ptr<Image> blend_image;
		int material_count = 0;
		std::shared_ptr<Material> materials[4];

		physx::PxRigidActor *actor = nullptr;

		Terrain(int _block_cx = 64, int _block_cy = 64, bool _use_physx = false, 
			std::shared_ptr<Image> _blendMap = nullptr);
		void add_material(std::shared_ptr<Material> m);
		void remove_material(int index);
	};
}
