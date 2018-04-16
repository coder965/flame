#pragma once

#include "descriptor.h"
#include "graphics_private.h"

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct DescriptorsetlayoutBinding
		{
			ShaderResourceType type;
			int binding;
			int count;
			int shader_stage;
		};

		struct DescriptorsetlayoutPrivate
		{
			std::vector<DescriptorsetlayoutBinding> bindings;

			Device *d;
			VkDescriptorSetLayout v;
		};

		struct DescriptorsetPrivate
		{
			Device *d;
			VkDescriptorSet v;
		};

		struct DescriptorpoolPrivate
		{
			Device *d;
			VkDescriptorPool v;
		};
	}
}
