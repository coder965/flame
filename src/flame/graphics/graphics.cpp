#include <vector>
#include <assert.h>

#include <flame/global.h>
#include <flame/system.h>

#include "graphics.h"
#include "graphics_private.h"

//#include <flame/global.h>
//#include <flame/engine/core/core.h>
//#include <flame/engine/graphics/graphics.h>
//#include <flame/engine/graphics/command_buffer.h>
//#include <flame/engine/graphics/buffer.h>
//#include <flame/engine/graphics/texture.h>
//#include <flame/engine/graphics/material.h>
//#include <flame/engine/graphics/descriptor.h>
//#include <flame/engine/graphics/renderpass.h>
//#include <flame/engine/graphics/pipeline.h>
//#include <flame/engine/graphics/shader.h>
//#include <flame/engine/graphics/sampler.h>
//#include <flame/engine/graphics/renderer.h>
//#include <flame/engine/graphics/pick_up.h>

namespace flame
{
	namespace graphics
	{
	}

	VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

	void vk_device_wait_idle()
	{
		//vkDeviceWaitIdle(g->_priv->device);
	}

	void vk_queue_submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore)
	{
	}

	const char *vk_device_type_names[] = {
		"other",
		"integrated gpu",
		"discrete gpu",
		"virtual gpu",
		"cpu"
	};
}
