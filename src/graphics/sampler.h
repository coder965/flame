#pragma once

#include "../refl.h"
#include "graphics.h"

namespace tke
{
	REFLECTABLE enum class SamplerType : int
	{
		null,
		REFLe none = 1 << 0,
		REFLe plain = 1 << 1,
		REFLe plain_unnormalized = 1 << 2,
		REFLe color = 1 << 3,
		REFLe color_border = 1 << 4
	};

	IMPL() VkSampler plainSampler;
	IMPL() VkSampler plainUnnormalizedSampler;
	IMPL() VkSampler colorSampler;
	IMPL() VkSampler colorBorderSampler;
	IMPL() VkSampler colorWrapSampler;

	void initSampler();
}
