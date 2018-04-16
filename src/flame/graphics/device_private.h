#include "device.h"
#include "graphics_private.h"

#include <flame/global.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate
		{
			std::list<std::function<void(int, int)>> resolution_change_listeners;

			bool resolution_change_event;

			VkInstance instance;
			VkPhysicalDevice physical_device;
			VkPhysicalDeviceProperties physical_device_properties;
			VkPhysicalDeviceFeatures physical_device_features; 
			VkPhysicalDeviceMemoryProperties mem_properties;
			VkDevice device;

			inline int find_memory_type(uint typeFilter, VkMemoryPropertyFlags properties)
			{
				for (uint i = 0; i < mem_properties.memoryTypeCount; i++)
				{
					if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
						return i;
				}
				return -1;
			}
		};
	}
}
