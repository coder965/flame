#pragma once

#include "UI.h"

namespace flame
{
	namespace graphics
	{
		struct Renderpass;
		struct Shader;
		struct Pipeline;
		struct Descriptorset;
		struct Textureview;
		struct Sampler;
		struct Buffer;
	}

	namespace UI
	{
		struct InstancePrivate
		{
			graphics::Renderpass *rp_clear;
			graphics::Renderpass *rp_not_clear;
			graphics::Shader *vert;
			graphics::Shader *frag;
			graphics::Pipeline *pl;
			graphics::Descriptorset *ds;
			graphics::Texture *font_tex;
			graphics::Textureview *font_view;
			graphics::Sampler *font_sam;
			graphics::Buffer *vtx_buffer;
			graphics::Buffer *idx_buffer;
		};
	}
}

