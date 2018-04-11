#include "device_private.h"
#include "semaphore_private.h"

namespace flame
{
	namespace graphics
	{
		Semaphore *create_semaphore(Device *d)
		{
			auto s = new Semaphore;
			s->_priv = new SemaphorePrivate;

			VkSemaphoreCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkSemaphore semaphore;
			vk_chk_res(vkCreateSemaphore(d->_priv->device, &info, nullptr, &s->_priv->v));

			return s;
		}

		void destroy_semaphore(Device *d, Semaphore *s)
		{
			vkDestroySemaphore(d->_priv->device, s->_priv->v, nullptr);

			delete s->_priv;
			delete s;
		}
	}
}

