#pragma once

#include "pipeline.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct Renderpass;

		struct PipelinePrivate
		{
			Device *d;
			Renderpass *renderpass;
			int subpass_index;

			VkPipeline v;
		};
	}
}
