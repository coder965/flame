#pragma once

#include "UI.h"

namespace flame
{
	namespace graphics
	{
		struct Shader;
		struct Pipeline;
		struct Descriptorset;
		struct Texture;
		struct Textureview;
		struct Sampler;
		struct Buffer;
	}

	namespace UI
	{
		struct InstancePrivate
		{
			graphics::Device *d;
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

