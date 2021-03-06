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
#include "swapchain_private.h"
#include "semaphore_private.h"

#include <flame/type.h>
#include <flame/system.h>

namespace flame
{
	namespace graphics
	{
#if defined(FLAME_GRAPHICS_VULKAN)
		int Swapchain::acquire_image(Semaphore *signal_semaphore)
		{
			uint index;
			vk_chk_res(vkAcquireNextImageKHR(_priv->d->_priv->device, _priv->swapchain, UINT64_MAX, signal_semaphore->_priv->v, VK_NULL_HANDLE, &index));
			return index;
		}

		Swapchain *create_swapchain(Device *d, void *win32_hwnd, const Ivec2 &size)
		{
			auto s = new Swapchain;
			s->size = size;

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

			unsigned int present_mode_count = 0;
			std::vector<VkPresentModeKHR> present_modes;
			vkGetPhysicalDeviceSurfacePresentModesKHR(d->_priv->physical_device, s->_priv->surface, &present_mode_count, nullptr);
			present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(d->_priv->physical_device, s->_priv->surface, &present_mode_count, present_modes.data());

			VkSurfaceCapabilitiesKHR surface_capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d->_priv->physical_device, s->_priv->surface, &surface_capabilities);
			assert(size.x >= surface_capabilities.minImageExtent.width);
			assert(size.y >= surface_capabilities.minImageExtent.height);
			assert(size.x <= surface_capabilities.maxImageExtent.width);
			assert(size.y <= surface_capabilities.maxImageExtent.height);

			unsigned int surface_format_count = 0;
			std::vector<VkSurfaceFormatKHR> surface_formats;
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->_priv->physical_device, s->_priv->surface,
				&surface_format_count, nullptr);
			assert(surface_format_count > 0);
			surface_formats.resize(surface_format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->_priv->physical_device, s->_priv->surface,
				&surface_format_count, surface_formats.data());

			s->format = Z(surface_formats[0].format, true);

			VkSwapchainCreateInfoKHR swapchain_info;
			swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchain_info.flags = 0;
			swapchain_info.pNext = nullptr;
			swapchain_info.surface = s->_priv->surface;
			swapchain_info.minImageCount = 2;
			swapchain_info.imageFormat = Z(s->format);
			swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
			swapchain_info.imageExtent.width = size.x;
			swapchain_info.imageExtent.height = size.y;
			swapchain_info.imageArrayLayers = 1;
			swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_info.queueFamilyIndexCount = 0;
			swapchain_info.pQueueFamilyIndices = nullptr;
			swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchain_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			swapchain_info.clipped = true;
			swapchain_info.oldSwapchain = 0;
			vk_chk_res(vkCreateSwapchainKHR(d->_priv->device, &swapchain_info, nullptr, &s->_priv->swapchain));

			uint image_count = 0;
			vkGetSwapchainImagesKHR(d->_priv->device, s->_priv->swapchain, &image_count, nullptr);
			vkGetSwapchainImagesKHR(d->_priv->device, s->_priv->swapchain, &image_count, s->_priv->images);

			for (int i = 0; i < 2; i++)
				s->_priv->image_views[i] = create_imageview(d, s->_priv->images[i], Z(s->format), VK_IMAGE_ASPECT_COLOR_BIT);

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
#endif
	}
}

