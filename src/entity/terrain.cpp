#include "../global.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/material.h"
#include "../graphics/pipeline.h"
#include "terrain.h"

namespace tke
{
	TerrainComponent::TerrainComponent() :
		Component(ComponentTypeTerrain),
		block_cx(64), 
		block_cy(64), 
		block_size(16.f),
		height(100.f),
		displacement_height(1.f),
		tessellation_factor(0.75f),
		tiling_scale(8.f),
		blend_image(default_blend_image),
		material_count(0),
		enable_physics(false),
		actor(nullptr),
		terrain_index(-1),
		attribute_dirty_frame(0),
		blend_image_dirty_frame(0)
	{
		add_material(default_material);
	}

	int TerrainComponent::get_block_cx() const
	{
		return block_cx;
	}

	int TerrainComponent::get_block_cy() const
	{
		return block_cy;
	}

	float TerrainComponent::get_block_size() const
	{
		return block_size;
	}

	float TerrainComponent::get_height() const
	{
		return height;
	}

	float TerrainComponent::get_displacement_height() const
	{
		return displacement_height;
	}

	float TerrainComponent::get_tessellation_factor() const
	{
		return tessellation_factor;
	}

	float TerrainComponent::get_tiling_scale() const
	{
		return tiling_scale;
	}

	Image *TerrainComponent::get_blend_image() const
	{
		return blend_image.get();
	}

	int TerrainComponent::get_material_count() const
	{
		return material_count;
	}

	Material *TerrainComponent::get_material(int index) const
	{
		if (index < material_count)
			return materials[index].get();
		return nullptr;
	}

	int TerrainComponent::get_terrain_index() const
	{
		return terrain_index;
	}

	void TerrainComponent::set_block_cx(int v)
	{
		block_cx = v;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_block_cy(int v)
	{
		block_cy = v;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_block_size(float v)
	{
		block_size = v;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_height(float v)
	{
		height = v;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_displacement_height(float v)
	{
		displacement_height = v;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_tessellation_factor(float v)
	{
		tessellation_factor = v;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_tiling_scale(float v)
	{
		tiling_scale = v;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_blend_image(std::shared_ptr<Image> i)
	{
		blend_image = i;
		blend_image_dirty_frame = total_frame_count;
	}

	void TerrainComponent::add_material(std::shared_ptr<Material> m)
	{
		if (material_count == TK_ARRAYSIZE(materials))
			return;

		materials[material_count] = m;
		material_count++;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::remove_material(int index)
	{
		if (index >= material_count)
			return;

		for (int i = index; i < material_count - 1; i++)
			materials[i] = materials[i + 1];
		material_count--;
		materials[material_count].reset();
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::set_terrain_index(int v)
	{
		terrain_index = v;
	}

	long long TerrainComponent::get_attribute_dirty_frame() const
	{
		return attribute_dirty_frame;
	}

	long long TerrainComponent::get_blend_image_dirty_frame() const
	{
		return blend_image_dirty_frame;
	}
}
