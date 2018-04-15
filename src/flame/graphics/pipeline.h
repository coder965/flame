#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct PipelinePrivate;

		struct Pipeline
		{
			PipelinePrivate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Pipeline *create_pipeline(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_pipeline(Device *d, Pipeline *p);
	}
}
