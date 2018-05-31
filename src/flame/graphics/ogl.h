#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Texture;
		struct Pipeline;
		struct Vao;

#if !defined(FLAME_GRAPHICS_VULKAN)
		FLAME_GRAPHICS_EXPORTS int ogl_get_error();

		FLAME_GRAPHICS_EXPORTS int ogl_get_active_texture();

		FLAME_GRAPHICS_EXPORTS void ogl_init();
		FLAME_GRAPHICS_EXPORTS void ogl_clear();

		FLAME_GRAPHICS_EXPORTS void ogl_active_texture(int id);

		FLAME_GRAPHICS_EXPORTS void ogl_bind_vao(Vao *v);
		FLAME_GRAPHICS_EXPORTS void ogl_bind_texture(Texture *t);

		FLAME_GRAPHICS_EXPORTS void ogl_uniform_int(int location, int v);
		FLAME_GRAPHICS_EXPORTS void ogl_uniform_mat4(int location, const Mat4 &v);

		FLAME_GRAPHICS_EXPORTS void ogl_viewport(int width, int height);
		FLAME_GRAPHICS_EXPORTS void ogl_scissor(int x, int y, int width, int height);

		FLAME_GRAPHICS_EXPORTS void ogl_draw_elements(int count, IndiceType idx_type, void *offset);

		FLAME_GRAPHICS_EXPORTS void ogl_fast_set(Pipeline *p);
		FLAME_GRAPHICS_EXPORTS void ogl_fast_reset();

#endif
	}
}
