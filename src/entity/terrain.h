#pragma once

#include "../graphics/graphics.h"
#include "component.h"

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

	class TerrainComponent : public Component
	{
	private:
		int block_cx;
		int block_cy;
		float block_size;
		float height;
		float displacement_height;
		float tessellation_factor;
		float tiling_scale;

		std::shared_ptr<Image> blend_image;

		int material_count;
		std::shared_ptr<Material> materials[4];

		bool enable_physics;

		std::unique_ptr<TerrainVertex[]> vertex;
		std::unique_ptr<VertexBuffer> vertex_buffer;

		physx::PxRigidActor *actor;

		int terrain_index;

		bool attribute_dirty;
		bool blend_image_dirty;
	public:
		TerrainComponent(Node *_parent);

		int get_block_cx() const;
		int get_block_cy() const;
		float get_block_size() const;
		float get_height() const;
		float get_displacement_height() const;
		float get_tessellation_factor() const;
		float get_tiling_scale() const;
		Image *get_blend_image() const;
		int get_material_count() const;
		Material *get_material(int index) const;
		int get_terrain_index() const;

		void set_block_cx(int v);
		void set_block_cy(int v);
		void set_block_size(float v);
		void set_height(float v);
		void set_displacement_height(float v);
		void set_tessellation_factor(float v);
		void set_tiling_scale(float v);
		void set_blend_image(std::shared_ptr<Image> i);
		void add_material(std::shared_ptr<Material> m);
		void remove_material(int index);
		void set_terrain_index(int v);

		bool is_attribute_dirty() const;
		bool is_blend_image_dirty() const;
		void clear_attribute_dirty();
		void clear_blend_image_dirty();
	};
}
