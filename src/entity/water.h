#pragma once

#include "node.h"

namespace tke
{
	class Water : public Node
	{
	private:
		int block_cx = 64;
		int block_cy = 64;
		float block_size = 16.f;
		float height = 10.f;
		float tessellation_factor = 0.75f;
		float tiling_scale = 8.f;
	public:
		Water();

		int get_block_cx() const;
		int get_block_cy() const;
		float get_block_size() const;
		float get_height() const;
		float get_tessellation_factor() const;
		float get_tiling_scale() const;

		void set_block_cx(int v);
		void set_block_cy(int v);
		void set_block_size(float v);
		void set_height(float v);
		void set_tessellation_factor(float v);
		void set_tiling_scale(float v);
	};
}
