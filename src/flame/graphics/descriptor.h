#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Pipeline;
		struct Buffer;
		struct Texture;
		struct Sampler;

		struct DescriptorsetlayoutPrivate;

		struct Descriptorsetlayout
		{
			DescriptorsetlayoutPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void clear_bindings();
			FLAME_GRAPHICS_EXPORTS void add_binding(ShaderResourceType type, int binding, int count, ShaderType shader_type);
			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Descriptorsetlayout *create_descriptorsetlayout(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_descriptorsetlayout(Device *d, Descriptorsetlayout *l);

		struct DescriptorsetPrivate;

		struct Descriptorset
		{
			DescriptorsetPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void set_uniformbuffer(int binding, int index, Buffer *b, int offset = 0, int range = 0);
			FLAME_GRAPHICS_EXPORTS void set_texture(Texture *t, Sampler sampler, int base_level = 0, int level_count = 0, 
				int base_layer = 0, int layer_count = 0, TextureViewType view_type = TextureViewType2D);
		};

		struct DescriptorpoolPrivate;

		struct Descriptorpool
		{
			DescriptorpoolPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS Descriptorset *create_descriptorset(Descriptorsetlayout *l);
			FLAME_GRAPHICS_EXPORTS Descriptorset *create_descriptorset(Pipeline *p, int index);
			FLAME_GRAPHICS_EXPORTS void destroy_descriptorset(Descriptorset *s);
		};

		FLAME_GRAPHICS_EXPORTS Descriptorpool *create_descriptorpool(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_descriptorpool(Device *d, Descriptorpool *p);
	}
}

