#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Commandpool;
		struct Queue;
		struct Descriptorpool;
		struct DevicePrivate;

		struct Device
		{
			bool features_texture_compression_BC;
			bool features_texture_compression_ASTC;
			bool features_texture_compression_ETC2;

			Commandpool *cp;
			Queue *q;
			Descriptorpool *dp;

			DevicePrivate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Device *create_device(bool enable_debug);
		FLAME_GRAPHICS_EXPORTS void destroy_device(Device *d);
	}
}

