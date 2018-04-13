#include "device_private.h"
#include "queue_private.h"
#include "semaphore_private.h"
#include "commandbuffer_private.h"
#include "swapchain_private.h"

namespace flame
{
	namespace graphics
	{
		void Queue::wait_idle()
		{
			vk_chk_res(vkQueueWaitIdle(_priv->v));
		}

		void Queue::submit(Commandbuffer *c, Semaphore *wait_semaphore, Semaphore *signal_semaphore)
		{
			VkSubmitInfo info;
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.pNext = nullptr;
			VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			info.pWaitDstStageMask = &waitStage;
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
	}
}
