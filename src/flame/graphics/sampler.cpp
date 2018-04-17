#include "sampler_private.h"
#include "device_private.h"

namespace flame
{
	namespace graphics
	{
		Sampler *create_sampler(Device *d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			auto s = new Sampler;

			s->_priv = new SamplerPirvate;

			VkSamplerCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.magFilter = Z(mag_filter);
			info.minFilter = Z(min_filter);
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.anisotropyEnable = VK_FALSE;
			info.maxAnisotropy = 1.f;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			info.unnormalizedCoordinates = unnormalized_coordinates;
			info.compareEnable = VK_FALSE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.mipLodBias = 0.0f;
			info.minLod = 0.0f;
			info.maxLod = 0.0f;

			vk_chk_res(vkCreateSampler(d->_priv->device, &info, nullptr, &s->_priv->v));

			return s;
		}

		void destroy_sampler(Device *d, Sampler *s)
		{
			vkDestroySampler(d->_priv->device, s->_priv->v, nullptr);

			delete s->_priv;
			delete s;
		}
	}
}

