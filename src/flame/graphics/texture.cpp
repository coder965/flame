//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <vector>

#include "texture_private.h"
#include "device_private.h"
#include "buffer_private.h"
#include "commandbuffer_private.h"
#include "queue_private.h"

#include <flame/filesystem.h>
#include <flame/image.h>
#include <gli/gli.hpp>

namespace flame
{
	namespace graphics
	{
#if !defined(FLAME_GRAPHICS_VULKAN)
		void Texture::image2D(Format _format, const Ivec2 &_size, void *data)
		{
			format = _format;
			size = _size;
			glBindTexture(GL_TEXTURE_2D, _priv->v);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			GLuint type, fmt;
			external_format(_format, fmt, type);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format(_format), _size.x, _size.y, 0, fmt, type, data);
		}
#endif

#if defined(FLAME_GRAPHICS_VULKAN)
		Texture *create_texture(Device *d, const Ivec2 &size, int level, int layer, Format format, int usage, int mem_prop)
		{
			auto t = new Texture;
			t->size = size;
			t->level = level;
			t->layer = layer;
			t->format = format;

			t->_priv = new TexturePrivate;
			t->_priv->usage = usage;
			t->_priv->mem_prop = mem_prop;
			t->_priv->d = d;

			VkImageCreateInfo imageInfo;
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.flags = 0;
			imageInfo.pNext = nullptr;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = size.x;
			imageInfo.extent.height = size.y;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = level;
			imageInfo.arrayLayers = layer;
			imageInfo.format = Z(t->format);
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = Z((TextureUsage)usage, format);
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.queueFamilyIndexCount = 0;
			imageInfo.pQueueFamilyIndices = nullptr;

			vk_chk_res(vkCreateImage(d->_priv->device, &imageInfo, nullptr, &t->_priv->v));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(d->_priv->device, t->_priv->v, &memRequirements);

			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = d->_priv->find_memory_type(memRequirements.memoryTypeBits, Z((MemProp)mem_prop));

			vk_chk_res(vkAllocateMemory(d->_priv->device, &allocInfo, nullptr, &t->_priv->m));

			vk_chk_res(vkBindImageMemory(d->_priv->device, t->_priv->v, t->_priv->m, 0));

			return t;
		}

		Texture *create_texture_from_file(Device *d, const std::string &filename, int usage, int mem_prop)
		{
			std::filesystem::path path(filename);
			if (!std::filesystem::exists(path))
				return nullptr;

			int width, height, level, layer;
			auto fmt = Format(-1);

			Buffer *staging_buffer;
			std::vector<BufferImageCopy> buffer_copy_regions;

			auto ext = path.extension().string();
			if (ext == ".ktx" || ext == ".dds")
			{
				gli::gl GL(gli::gl::PROFILE_GL33);

				auto gli_texture = gli::load(filename);
				if (gli_texture.empty())
					assert(0);

				assert(gli_texture.target() == gli::TARGET_2D);
				auto const gli_format = GL.translate(gli_texture.format(), gli_texture.swizzles());

				width = gli_texture.extent().x;
				height = gli_texture.extent().y;
				level = gli_texture.levels();
				layer = gli_texture.layers();

				switch (gli_format.Internal)
				{
					case gli::gl::INTERNAL_RGBA_DXT5:
						fmt = Format_RGBA_BC3;
						break;
					case gli::gl::INTERNAL_RGBA_ETC2:
						fmt = Format_RGBA_ETC2;
						break;
				}

				staging_buffer = create_buffer(d, gli_texture.size(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, gli_texture.data(), staging_buffer->size);
				staging_buffer->unmap();

				auto offset = 0;
				for (auto i = 0; i < level; i++)
				{
					BufferImageCopy c;
					c.buffer_offset = offset;
					c.image_width = gli_texture.extent(i).x;
					c.image_height = gli_texture.extent(i).y;
					c.image_level = i;
					buffer_copy_regions.push_back(c);
					offset += gli_texture.size(i);
				}
			}
			else
			{
				auto image = load_image(filename);
				if (image->channel == 3)
					image->add_alpha_channel();

				width = image->cx;
				height = image->cy;
				level = layer = 1;

				switch (image->channel)
				{
					case 0:
						switch (image->bpp)
						{
							case 8:
								fmt = Format_R8_UNORM;
								break;
						}
						break;
					case 1:
						switch (image->bpp)
						{
							case 8:
								fmt = Format_R8_UNORM;
								break;
							case 16:
								fmt = Format_R16_UNORM;
								break;
						}
						break;
					case 4:
						switch (image->bpp)
						{
							case 32:
								fmt = Format_R8G8B8A8_UNORM;
								break;
						}
						break;
				}

				staging_buffer = create_buffer(d, image->size, BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, image->data, staging_buffer->size);
				staging_buffer->unmap();

				release_image(image);

				{
					BufferImageCopy c;
					c.buffer_offset = 0;
					c.image_width = width;
					c.image_height = height;
					c.image_level = 0;
					buffer_copy_regions.push_back(c);
				}
			}

			auto t = create_texture(d, Ivec2(width, height), level, layer, fmt, usage |
				TextureUsageShaderSampled | TextureUsageTransferDst, mem_prop | MemPropDevice);

			auto cb = d->cp->create_commandbuffer();
			cb->begin(true);
			cb->change_texture_layout(t, TextureLayoutUndefined, TextureLayoutTransferDst);
			cb->copy_buffer_to_image(staging_buffer, t, buffer_copy_regions.size(), buffer_copy_regions.data());
			cb->change_texture_layout(t, TextureLayoutTransferDst, TextureLayoutShaderReadOnly);
			cb->end();
			d->q->submit(cb, nullptr, nullptr);
			d->q->wait_idle();
			d->cp->destroy_commandbuffer(cb);
			destroy_buffer(d, staging_buffer);

			return t;
		}
#else
		Texture *create_texture(Device *d)
		{
			auto t = new Texture;
			t->size = Ivec2(0);
			t->level = 1;
			t->layer = 1;
			t->format = Format_Undefined;

			t->_priv = new TexturePrivate;
			glGenTextures(1, &t->_priv->v);

			return t;
		}
#endif
		void destroy_texture(Device *d, Texture *t)
		{
#if defined(FLAME_GRAPHICS_VULKAN)
			vkFreeMemory(d->_priv->device, t->_priv->m, nullptr);
			vkDestroyImage(d->_priv->device, t->_priv->v, nullptr);
#else
			glDeleteTextures(1, &t->_priv->v);
#endif
			delete t->_priv;
			delete t;
		}

#if defined(FLAME_GRAPHICS_VULKAN)
		Textureview *create_textureview(Device *d, Texture *t,
			TextureViewType view_type, int base_level, int level_count, int base_layer, int layer_count)
		{
			auto v = new Textureview;

			v->_priv = new TextureviewPrivate;

			v->_priv->v = create_imageview(d, t->_priv->v, Z(t->format), Z(format_to_aspect(t->format)), Z(view_type),
				base_level, level_count, base_layer, layer_count);

			return v;
		}

		void destroy_textureview(Device *d, Textureview *v)
		{
			destroy_imageview(d, v->_priv->v);

			delete v->_priv;
			delete v;
		}
#endif
	}
}

