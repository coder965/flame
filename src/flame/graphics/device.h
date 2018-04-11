#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct Device
		{
			float near_plane;
			float far_plane;
			float fovy;

			int resolution_x;
			int resolution_y;
			float aspect;

			DevicePrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void set_resolution(int x, int y); // 0 means keep
			FLAME_GRAPHICS_EXPORTS void *add_resolution_change_listener(const std::function<void(int, int)> &e);
			FLAME_GRAPHICS_EXPORTS void remove_resolution_change_listener(void *p);
		};

		FLAME_GRAPHICS_EXPORTS Device *create_device(bool enable_debug, int _resolution_x, int _resolution_y);
		FLAME_GRAPHICS_EXPORTS void destroy_device(Device *d);
	}
}

