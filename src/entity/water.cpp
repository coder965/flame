#include "water.h"

namespace tke
{
	Water::Water()
		:Node(NodeTypeWater)
	{
	}

	int Water::get_block_cx() const
	{
		return block_cx;
	}

	int Water::get_block_cy() const
	{
		return block_cy;
	}

	float Water::get_block_size() const
	{
		return block_size;
	}

	float Water::get_height() const
	{
		return height;
	}

	float Water::get_tessellation_factor() const
	{
		return tessellation_factor;
	}

	float Water::get_tiling_scale() const
	{
		return tiling_scale;
	}

	void Water::set_block_cx(int v)
	{
		block_cx = v;
		attribute_dirty = true;
	}

	void Water::set_block_cy(int v)
	{
		block_cy = v;
		attribute_dirty = true;
	}

	void Water::set_block_size(float v)
	{
		block_size = v;
		attribute_dirty = true;
	}

	void Water::set_height(float v)
	{
		height = v;
		attribute_dirty = true;
	}

	void Water::set_tessellation_factor(float v)
	{
		tessellation_factor = v;
		attribute_dirty = true;
	}

	void Water::set_tiling_scale(float v)
	{
		tiling_scale = v;
		attribute_dirty = true;
	}
}
