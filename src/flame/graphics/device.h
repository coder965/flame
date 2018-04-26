#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct Device
		{
			bool features_texture_compression_BC;
			bool features_texture_compression_ASTC;
			bool features_texture_compression_ETC2;

			DevicePrivate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Device *create_device(bool enable_debug);
		FLAME_GRAPHICS_EXPORTS void destroy_device(Device *d);
	}
}

