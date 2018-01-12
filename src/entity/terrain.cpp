#include "../global.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/material.h"
#include "../graphics/pipeline.h"
#include "terrain.h"

namespace tke
{
	Terrain::Terrain(Node *_parent) :
		Component(parent, ComponentTypeTerrain),
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
		attribute_dirty(true),
		blend_image_dirty(true)
	{
		add_material(defaultMaterial);
	}

	int Terrain::get_block_cx() const
	{
		return block_cx;
	}

	int Terrain::get_block_cy() const
	{
		return block_cy;
	}

	float Terrain::get_block_size() const
	{
		return block_size;
	}

	float Terrain::get_height() const
	{
		return height;
	}

	float Terrain::get_displacement_height() const
	{
		return displacement_height;
	}

	float Terrain::get_tessellation_factor() const
	{
		return tessellation_factor;
	}

	float Terrain::get_tiling_scale() const
	{
		return tiling_scale;
	}

	Image *Terrain::get_blend_image() const
	{
		return blend_image.get();
	}

	int Terrain::get_material_count() const
	{
		return material_count;
	}

	Material *Terrain::get_material(int index) const
	{
		if (index < material_count)
			return materials[index].get();
		return nullptr;
	}

	void Terrain::set_block_cx(int v)
	{
		block_cx = v;
		attribute_dirty = true;
	}

	void Terrain::set_block_cy(int v)
	{
		block_cy = v;
		attribute_dirty = true;
	}

	void Terrain::set_block_size(float v)
	{
		block_size = v;
		attribute_dirty = true;
	}

	void Terrain::set_height(float v)
	{
		height = v;
		attribute_dirty = true;
	}

	void Terrain::set_displacement_height(float v)
	{
		displacement_height = v;
		attribute_dirty = true;
	}

	void Terrain::set_tessellation_factor(float v)
	{
		tessellation_factor = v;
		attribute_dirty = true;
	}

	void Terrain::set_tiling_scale(float v)
	{
		tiling_scale = v;
		attribute_dirty = true;
	}

	void Terrain::set_blend_image(std::shared_ptr<Image> i)
	{
		blend_image = i;
		blend_image_dirty = true;
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

	bool Terrain::is_attribute_dirty() const
	{
		return attribute_dirty;
	}

	bool Terrain::is_blend_image_dirty() const
	{
		return blend_image_dirty;
	}

	void Terrain::clear_attribute_dirty()
	{
		attribute_dirty = false;
	}

	void Terrain::clear_blend_image_dirty()
	{
		blend_image_dirty = false;
	}
}
