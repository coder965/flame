#include "device_private.h"
#include "swapchain_private.h"
#include "semaphore_private.h"

#include <flame/global.h>
#include <flame/system.h>

namespace flame
{
	namespace graphics
	{
		int Swapchain::acquire_image(Semaphore *signal_semaphore)
		{
			uint index;
			vk_chk_res(vkAcquireNextImageKHR(_priv->d->_priv->device, _priv->swapchain, UINT64_MAX, signal_semaphore->_priv->v, VK_NULL_HANDLE, &index));
			return index;
		}

		Swapchain *create_swapchain(Device *d, void *win32_hwnd, int cx, int cy)
		{
			auto s = new Swapchain;
			s->cx = cx;
			s->cy = cy;

			s->_priv = new SwapchainPrivate;
			s->_priv->d = d;

			VkWin32SurfaceCreateInfoKHR surface_info;
			surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_info.flags = 0;
			surface_info.pNext = nullptr;
			surface_info.hinstance = (HINSTANCE)get_hinst();
			surface_info.hwnd = (HWND)win32_hwnd;
			vk_chk_res(vkCreateWin32SurfaceKHR(d->_priv->instance, &surface_info, nullptr, &s->_priv->surface));

			VkBool32 surface_supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(d->_priv->physical_device, 0, s->_priv->surface, &surface_supported);
			assert(surface_supported);

			VkSurfaceCapabilitiesKHR surface_capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d->_priv->physical_device, s->_priv->surface, &surface_capabilities);
			assert(cx >= surface_capabilities.minImageExtent.width);
			assert(cy >= surface_capabilities.minImageExtent.height);
			assert(cx <= surface_capabilities.maxImageExtent.width);
			assert(cy <= surface_capabilities.maxImageExtent.height);

			unsigned int surface_format_count = 0;
			std::vector<VkSurfaceFormatKHR> surface_formats;
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->_priv->physical_device, s->_priv->surface,
				&surface_format_count, nullptr);
			assert(surface_format_count > 0);
			surface_formats.resize(surface_format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->_priv->physical_device, s->_priv->surface,
				&surface_format_count, surface_formats.data());

			s->_priv->format = surface_formats[0].format;

			VkSwapchainCreateInfoKHR swapchain_info;
			swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchain_info.flags = 0;
			swapchain_info.pNext = nullptr;
			swapchain_info.surface = s->_priv->surface;
			swapchain_info.minImageCount = 2;
			swapchain_info.imageFormat = s->_priv->format;
			swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
			swapchain_info.imageExtent.width = cx;
			swapchain_info.imageExtent.height = cy;
			swapchain_info.imageArrayLayers = 1;
			swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_info.queueFamilyIndexCount = 0;
			swapchain_info.pQueueFamilyIndices = nullptr;
			swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			swapchain_info.clipped = true;
			swapchain_info.oldSwapchain = 0;
			vk_chk_res(vkCreateSwapchainKHR(d->_priv->device, &swapchain_info, nullptr, &s->_priv->swapchain));

			uint image_count = 0;
			vkGetSwapchainImagesKHR(d->_priv->device, s->_priv->swapchain, &image_count, nullptr);
			vkGetSwapchainImagesKHR(d->_priv->device, s->_priv->swapchain, &image_count, s->_priv->images);

			for (int i = 0; i < 2; i++)
				s->_priv->image_views[i] = create_imageview(d, s->_priv->images[i], s->_priv->format, VK_IMAGE_ASPECT_COLOR_BIT);

			return s;
		}

		void destroy_swapchain(Device *d, Swapchain *s)
		{
			for (int i = 0; i < 2; i++)
				destroy_imageview(d, s->_priv->image_views[i]);
			vkDestroySwapchainKHR(d->_priv->device, s->_priv->swapchain, nullptr);
			vkDestroySurfaceKHR(d->_priv->instance, s->_priv->surface, nullptr);

			delete s->_priv;
			delete s;
		}
	}
}

