#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct SamplerPirvate;

		struct Sampler
		{
			SamplerPirvate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Sampler *create_sampler(Device *d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
		FLAME_GRAPHICS_EXPORTS void destroy_sampler(Device *d, Sampler *s);

	}
}

