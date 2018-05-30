//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "device_private.h"
#include "queue_private.h"
#include "semaphore_private.h"
#include "commandbuffer_private.h"
#include "swapchain_private.h"

namespace flame
{
	namespace graphics
	{
#if defined(FLAME_GRAPHICS_VULKAN)
		void Queue::wait_idle()
		{
			vk_chk_res(vkQueueWaitIdle(_priv->v));
		}

		void Queue::submit(Commandbuffer *c, Semaphore *wait_semaphore, Semaphore *signal_semaphore)
		{
			VkSubmitInfo info;
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.pNext = nullptr;
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			info.pWaitDstStageMask = &wait_stage;
			info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
			info.pWaitSemaphores = wait_semaphore ? &wait_semaphore->_priv->v : nullptr;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &c->_priv->v;
			info.signalSemaphoreCount = signal_semaphore ? 1 : 0;
			info.pSignalSemaphores = signal_semaphore  ? &signal_semaphore->_priv->v : nullptr;

			vk_chk_res(vkQueueSubmit(_priv->v, 1, &info, VK_NULL_HANDLE));
		}

		void Queue::present(uint index, Swapchain *s, Semaphore *wait_semaphore)
		{
			VkPresentInfoKHR present_info;
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pNext = nullptr;
			present_info.pResults = nullptr;
			present_info.waitSemaphoreCount = 1;
			present_info.pWaitSemaphores = &wait_semaphore->_priv->v;
			present_info.swapchainCount = 1;
			present_info.pSwapchains = &s->_priv->swapchain;
			present_info.pImageIndices = &index;
			vk_chk_res(vkQueuePresentKHR(_priv->v, &present_info));
		}

		static int queue_count = 1;

		Queue *create_queue(Device *d)
		{
			assert(queue_count > 0);

			auto q = new Queue;

			q->_priv = new QueuePrivate;
			q->_priv->d = d;

			vkGetDeviceQueue(d->_priv->device, 0, 0, &q->_priv->v);

			queue_count--;

			return q;
		}

		void destroy_queue(Device *d, Queue *q)
		{
			assert(d == q->_priv->d);

			delete q->_priv;
			delete q;

			queue_count++;;
		}
#endif
	}
}
