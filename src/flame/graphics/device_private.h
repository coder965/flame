#include "device.h"
#include "graphics_private.h"

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
			VkDevice device;
		};
	}
}
