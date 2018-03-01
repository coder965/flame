#include <flame/global.h>
#include <flame/entity/water.h>

namespace flame
{
	WaterComponent::WaterComponent() :
		Component(ComponentTypeWater),
		block_cx(64),
		block_cy(64),
		block_size(16.f),
		height(10.f),
		tessellation_factor(0.75f),
		tiling_scale(8.f),
		attribute_dirty_frame(0)
	{
	}

	int WaterComponent::get_block_cx() const
	{
		return block_cx;
	}

	int WaterComponent::get_block_cy() const
	{
		return block_cy;
	}

	float WaterComponent::get_block_size() const
	{
		return block_size;
	}

	float WaterComponent::get_height() const
	{
		return height;
	}

	float WaterComponent::get_tessellation_factor() const
	{
		return tessellation_factor;
	}

	float WaterComponent::get_tiling_scale() const
	{
		return tiling_scale;
	}

	int WaterComponent::get_water_index() const
	{
		return water_index;
	}

	void WaterComponent::set_block_cx(int v)
	{
		block_cx = v;
		attribute_dirty_frame = total_frame_count;
	}

	void WaterComponent::set_block_cy(int v)
	{
		block_cy = v;
		attribute_dirty_frame = total_frame_count;
	}

	void WaterComponent::set_block_size(float v)
	{
		block_size = v;
		attribute_dirty_frame = total_frame_count;
	}

	void WaterComponent::set_height(float v)
	{
		height = v;
		attribute_dirty_frame = total_frame_count;
	}

	void WaterComponent::set_tessellation_factor(float v)
	{
		tessellation_factor = v;
		attribute_dirty_frame = total_frame_count;
	}

	void WaterComponent::set_tiling_scale(float v)
	{
		tiling_scale = v;
		attribute_dirty_frame = total_frame_count;
	}

	void WaterComponent::set_water_index(int v)
	{
		water_index = v;
	}

	long long WaterComponent::get_attribute_dirty_frame() const
	{
		return attribute_dirty_frame;
	}
}
