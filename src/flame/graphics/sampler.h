#pragma once

#include <flame/graphics/graphics.h>

namespace tke
{
	enum class SamplerType : int
	{
		null,
		none = 1 << 0,
		plain = 1 << 1,
		plain_unnormalized = 1 << 2,
		color = 1 << 3,
		color_border = 1 << 4
	};

	extern VkSampler plainSampler;
	extern VkSampler plainUnnormalizedSampler;
	extern VkSampler colorSampler;
	extern VkSampler colorBorderSampler;
	extern VkSampler colorWrapSampler;

	void init_sampler();
}
