#include "UI_private.h"

#include <flame/system.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/queue.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/sampler.h>

#include <imgui.h>
#include <Windows.h>

namespace flame
{
	namespace UI
	{
		void Instance::begin()
		{

		}

		void Instance::end()
		{

		}

		void Instance::record_commandbuffer(graphics::Commandbuffer *cb)
		{

		}

		Instance *create_instance(graphics::Device *d, graphics::Descriptorpool *dp, graphics::Commandpool *cp, graphics::Queue *q)
		{
			auto i = new Instance;

			i->_priv = new InstancePrivate;

			i->_priv->rp_clear = graphics::create_renderpass(d);
			i->_priv->rp_clear->add_attachment(graphics::Format::R8G8B8A8, true);
			i->_priv->rp_clear->add_subpass({0}, -1);
			i->_priv->rp_clear->build();

			i->_priv->rp_not_clear = graphics::create_renderpass(d);
			i->_priv->rp_not_clear->add_attachment(graphics::Format::R8G8B8A8, false);
			i->_priv->rp_not_clear->add_subpass({0}, -1);
			i->_priv->rp_not_clear->build();

			i->_priv->vert = graphics::create_shader(d, "ui.vert");
			i->_priv->vert->build();
			i->_priv->frag = graphics::create_shader(d, "ui.frag");
			i->_priv->frag->build();

			i->_priv->pl = create_pipeline(d);
			i->_priv->pl->set_vertex_attributes({{
				graphics::VertexAttributeFloat2,
				graphics::VertexAttributeFloat2,
				graphics::VertexAttributeByte4
			}});
			i->_priv->pl->set_size(0, 0);
			i->_priv->pl->set_cull_mode(graphics::CullModeNone);
			i->_priv->pl->set_blend_state(0, true, graphics::BlendFactorSrcAlpha, graphics::BlendFactorOneMinusSrcAlpha,
				graphics::BlendFactorZero, graphics::BlendFactorOneMinusSrcAlpha);
			i->_priv->pl->set_dynamic_state({graphics::DynamicStateScissor});
			i->_priv->pl->add_shader(i->_priv->vert);
			i->_priv->pl->add_shader(i->_priv->frag);
			i->_priv->pl->set_renderpass(i->_priv->rp_clear, 0);
			i->_priv->pl->build_graphics();

			i->_priv->ds = dp->create_descriptorset(i->_priv->pl, 0);

			ImGui::CreateContext();
			auto &im_io = ImGui::GetIO();
			im_io.Fonts->AddFontDefault();
			unsigned char* font_pixels; int font_tex_width, font_tex_height;
			im_io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_tex_width, &font_tex_height);
			i->_priv->font_tex = graphics::create_texture(d, font_tex_width, font_tex_height, 
				1, 1, graphics::Format::R8G8B8A8, graphics::TextureUsageShaderSampled | 
				graphics::TextureUsageTransferDst, graphics::MemPropDevice);

			auto font_stag = graphics::create_buffer(d, font_tex_width * font_tex_height * 4, 
				graphics::BufferUsageTransferSrc, graphics::MemPropHost | graphics::MemPropHostCoherent);
			font_stag->map();
			memcpy(font_stag->mapped, font_pixels, font_stag->size);
			font_stag->unmap();

			auto cb_cpy = cp->create_commandbuffer();
			cb_cpy->begin(true);
			cb_cpy->change_texture_layout(i->_priv->font_tex, graphics::TextureLayoutUndefined, graphics::TextureLayoutTransferDst);
			graphics::BufferImageCopy font_cpy_range;
			font_cpy_range.buffer_offset = 0;
			font_cpy_range.image_width = font_tex_width;
			font_cpy_range.image_height = font_tex_height;
			font_cpy_range.image_level = 0;
			cb_cpy->copy_buffer_to_image(font_stag, i->_priv->font_tex, 1, &font_cpy_range);
			cb_cpy->change_texture_layout(i->_priv->font_tex, graphics::TextureLayoutTransferDst, graphics::TextureLayoutShaderReadOnly);
			cb_cpy->end();
			q->submit(cb_cpy, nullptr, nullptr);
			q->wait_idle();
			cp->destroy_commandbuffer(cb_cpy);

			graphics::destroy_buffer(d, font_stag);

			im_io.Fonts->TexID = (void*)0; // image index

			i->_priv->font_view = graphics::create_textureview(d, i->_priv->font_tex);

			i->_priv->font_sam = graphics::create_sampler(d, graphics::FilterLinear, graphics::FilterLinear, false);

			i->_priv->ds->set_texture(0, 0, i->_priv->font_view, i->_priv->font_sam);

			im_io.KeyMap[ImGuiKey_Tab] = VK_TAB;
			im_io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			im_io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			im_io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			im_io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			im_io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
			im_io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
			im_io.KeyMap[ImGuiKey_Home] = VK_HOME;
			im_io.KeyMap[ImGuiKey_End] = VK_END;
			im_io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			im_io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			im_io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			im_io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			im_io.KeyMap[ImGuiKey_A] = 'A';
			im_io.KeyMap[ImGuiKey_C] = 'C';
			im_io.KeyMap[ImGuiKey_V] = 'V';
			im_io.KeyMap[ImGuiKey_X] = 'X';
			im_io.KeyMap[ImGuiKey_Y] = 'Y';
			im_io.KeyMap[ImGuiKey_Z] = 'Z';
			im_io.SetClipboardTextFn = [](void *user_data, const char *s) {
				set_clipBoard(s);
			};
			im_io.GetClipboardTextFn = [](void *user_data) {
				static std::string s;
				s = get_clipBoard();
				return s.c_str();
			};

			return i;
		}

		void destroy_instance(graphics::Device *d, Instance *i)
		{
			graphics::destroy_renderpass(d, i->_priv->rp_clear);
			graphics::destroy_renderpass(d, i->_priv->rp_not_clear);
			graphics::destroy_shader(d, i->_priv->vert);
			graphics::destroy_shader(d, i->_priv->frag);
			graphics::destroy_pipeline(d, i->_priv->pl);

			delete i->_priv;
			delete i;
		}
	}
}

