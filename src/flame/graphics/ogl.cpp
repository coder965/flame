#include "ogl.h"
#include "graphics_private.h"
#include "texture_private.h"
#include "pipeline_private.h"
#include "vao_private.h"

#include <Windows.h>

namespace flame
{
	namespace graphics
	{
#if !defined(FLAME_GRAPHICS_VULKAN)
		int ogl_get_error()
		{
			return glGetError();
		}

		LRESULT CALLBACK DummyWndProc(HWND p0, UINT p1, WPARAM p2, LPARAM p3)
		{
			return DefWindowProc(p0, p1, p2, p3);
		}

		void ogl_init()
		{
			HINSTANCE hInstance = GetModuleHandle(NULL);

			WNDCLASS dummy_wc;
			memset(&dummy_wc, 0, sizeof(WNDCLASS));
			dummy_wc.hInstance = hInstance;
			dummy_wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			dummy_wc.lpfnWndProc = DummyWndProc;
			dummy_wc.lpszClassName = "DummyClass";
			if (!RegisterClass(&dummy_wc))
			{
				assert(0);
				return;
			}

			HWND dummy_hWnd = CreateWindow("DummyClass", "Dummy", WS_OVERLAPPEDWINDOW,
				0, 0, 100, 100, NULL, NULL, hInstance, NULL);
			if (!dummy_hWnd)
			{
				auto error = GetLastError();
				assert(0);
				return;
			}

			PIXELFORMATDESCRIPTOR dummy_pfd = {
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,
				24,
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				0,
				0,
				0,
				PFD_MAIN_PLANE,
				0,
				0, 0, 0
			};

			HDC dummy_hDC = GetDC(dummy_hWnd);
			int dummy_pf = ChoosePixelFormat(dummy_hDC, &dummy_pfd);
			SetPixelFormat(dummy_hDC, dummy_pf, &dummy_pfd);

			HGLRC dummy_hRC = wglCreateContext(dummy_hDC);
			HDC last_hDC = wglGetCurrentDC();
			HGLRC last_hRC = wglGetCurrentContext();
			wglMakeCurrent(dummy_hDC, dummy_hRC);

			glewInit();

			wglMakeCurrent(last_hDC, last_hRC);
			wglDeleteContext(dummy_hRC);
			ReleaseDC(dummy_hWnd, dummy_hDC);
			DestroyWindow(dummy_hWnd);
			UnregisterClass("DummyClass", hInstance);
		}

		void ogl_clear()
		{
			//glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		static GLint last_program;
		static GLint last_texture;
		static GLint last_array_buffer;
		static GLint last_element_array_buffer;
		static GLint last_vertex_array;
		static GLenum last_active_texture;
		static GLint last_polygon_mode[2];
		static GLint last_viewport[4];
		static GLint last_scissor_box[4];
		static GLenum last_blend_src_rgb;
		static GLenum last_blend_dst_rgb;
		static GLenum last_blend_src_alpha;
		static GLenum last_blend_dst_alpha;
		static GLenum last_blend_equation_rgb;
		static GLenum last_blend_equation_alpha; 
		static GLboolean last_enable_blend;
		static GLboolean last_enable_cull_face;
		static GLboolean last_enable_depth_test;
		static GLboolean last_enable_scissor_test;

		void ogl_fast_set(Pipeline *p)
		{
			glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
			glActiveTexture(GL_TEXTURE0);

			glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
			glGetIntegerv(GL_VIEWPORT, last_viewport);
			glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
			glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
			glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
			glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
			glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
			glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
			glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
			last_enable_blend = glIsEnabled(GL_BLEND);
			last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
			last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
			last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

			glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
			glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_SCISSOR_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUseProgram(p->_priv->v);
		}

		void ogl_fast_reset()
		{
			auto e = glGetError();

			glUseProgram(last_program);
			glActiveTexture(last_active_texture);
			glBindTexture(GL_TEXTURE_2D, last_texture);
			glBindVertexArray(last_vertex_array);
			glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
			glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
			glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
			if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
			if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
			if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
			if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
			glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
			glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
		}

		void ogl_bind_vao(Vao *v)
		{
			glBindVertexArray(v->_priv->v);
		}

		void ogl_bind_texture(Texture *t)
		{
			glBindTexture(GL_TEXTURE_2D, t->_priv->v);
		}

		void ogl_uniform_int(int location, int v)
		{
			glUniform1i(location, v);
		}

		void ogl_uniform_mat4(int location, const Mat4 &v)
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, &v[0][0]);
		}

		void ogl_viewport(int width, int height)
		{
			glViewport(0, 0, width, height);
		}

		void ogl_scissor(int x, int y, int width, int height)
		{
			glScissor(x, y, width, height);
		}

		void ogl_draw_elements(int count, IndiceType idx_type, void *offset)
		{
			glDrawElements(GL_TRIANGLES, count, idx_type == IndiceTypeUshort ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, offset);
		}
#endif
	}
}
