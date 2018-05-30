#include "graphics.h"

namespace flame
{
	namespace graphics
	{
#if !defined(FLAME_GRAPHICS_VULKAN)
		FLAME_GRAPHICS_EXPORTS void ogl_init();
		FLAME_GRAPHICS_EXPORTS void ogl_clear();
#endif
	}
}
