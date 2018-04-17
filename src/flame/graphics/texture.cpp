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
		static VkImageLayout get_vk_image_layout(TextureLayout l, Format::Type ft)
		{
			switch (l)
			{
				case TextureLayoutUndefined:
					return VK_IMAGE_LAYOUT_UNDEFINED;
				case TextureLayoutAttachment:
					return ft == Format::TypeColor ?
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				case TextureLayoutShaderReadOnlyOptimal:
					return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case TextureLayoutTransferSrc:
					return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				case TextureLayoutTransferDst:
					return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			}
		}

		void Texture::change_layout(Commandbuffer *cb, TextureLayout from, TextureLayout to,
			int base_level, int level_count, int base_layer, int layer_count)
		{
			level_count = level_count == 0 ? level : level_count;
			layer_count = layer_count == 0 ? layer : layer_count;

			auto ft = format.get_type();

			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.oldLayout = get_vk_image_layout(from, ft);
			barrier.newLayout = get_vk_image_layout(to, ft);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = _priv->v;
			VkImageAspectFlags aspect;
			switch (ft)
			{
				case Format::TypeColor:
					aspect = VK_IMAGE_ASPECT_COLOR_BIT;
					break;
				case Format::TypeDepth:
					aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
					break;
				case Format::TypeDepthStencil:
					aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
			}
			barrier.subresourceRange.aspectMask = aspect;
			barrier.subresourceRange.baseMipLevel = base_level;
			barrier.subresourceRange.levelCount = level_count;
			barrier.subresourceRange.baseArrayLayer = base_layer;
			barrier.subresourceRange.layerCount = layer_count;

			switch (barrier.oldLayout)
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:
					barrier.srcAccessMask = 0;
					break;
				case VK_IMAGE_LAYOUT_PREINITIALIZED:
					barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
					barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
					break;
			}

			switch (barrier.newLayout)
			{
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
					break;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					if (barrier.srcAccessMask == 0)
						barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
			}

			vkCmdPipelineBarrier(cb->_priv->v, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		Texture *create_texture(Device *d, int cx, int cy, int level, int layer, Format format, int usage, int mem_prop)
		{
			auto t = new Texture;
			t->cx = cx == -1 ? d->resolution_x : cx;
			t->cy = cy == -1 ? d->resolution_y : cy;
			t->level = level;
			t->layer = layer;
			t->format = format;
			t->_priv = new TexturePrivate;
			t->_priv->d = d;

			auto ft = format.get_type();
			
			VkImageUsageFlags vk_usage = 0;
			if (usage & TextureUsageTransferSrc)
				vk_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (usage & TextureUsageTransferDst)
				vk_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (usage & TextureUsageShaderSampled)
				vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (usage & TextureUsageAttachment)
			{
				if (ft == Format::TypeColor)
					vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				else
					vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}

			VkImageCreateInfo imageInfo;
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.flags = 0;
			imageInfo.pNext = nullptr;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = t->cx;
			imageInfo.extent.height = t->cy;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = level;
			imageInfo.arrayLayers = layer;
			imageInfo.format = Z(format);
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = vk_usage;
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

		Texture *create_texture_from_file(Device *d, Commandpool *cp, Queue *queue, const std::string &filename, int usage, int mem_prop)
		{
			std::filesystem::path path(filename);
			if (!std::filesystem::exists(path))
				return nullptr;

			int width, height, level, layer;
			Format format;
			format.v = Format::Value(-1);

			Buffer *staging_buffer;
			std::vector<VkBufferImageCopy> buffer_copy_regions;

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
						format.v = Format::RGBA_BC3;
						break;
					case gli::gl::INTERNAL_RGBA_ETC2:
						format.v = Format::RGBA_ETC2;
						break;
				}

				staging_buffer = create_buffer(d, gli_texture.size(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, gli_texture.data(), staging_buffer->size);
				staging_buffer->unmap();

				auto offset = 0;
				for (auto i = 0; i < level; i++)
				{
					VkBufferImageCopy r = {};
					r.bufferOffset = offset;
					r.imageExtent.width = gli_texture.extent(i).x;
					r.imageExtent.height = gli_texture.extent(i).y;
					r.imageExtent.depth = 1;
					r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					r.imageSubresource.mipLevel = i;
					r.imageSubresource.layerCount = 1;
					buffer_copy_regions.push_back(r);
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
								format.v = Format::R8;
								break;
						}
						break;
					case 1:
						switch (image->bpp)
						{
							case 8:
								format.v = Format::R8;
								break;
							case 16:
								format.v = Format::R16;
								break;
						}
						break;
					case 4:
						switch (image->bpp)
						{
							case 32:
								format.v = Format::R8G8B8A8;
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
					VkBufferImageCopy r = {};
					r.imageExtent.width = width;
					r.imageExtent.height = height;
					r.imageExtent.depth = 1;
					r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					r.imageSubresource.layerCount = 1;
					buffer_copy_regions.push_back(r);
				}
			}

			auto t = create_texture(d, width, height, level, layer, format, usage | 
				TextureUsageShaderSampled | TextureUsageTransferDst, mem_prop | MemPropDevice);

			auto cb = cp->create_commandbuffer();
			cb->begin(true);
			t->change_layout(cb, TextureLayoutUndefined, TextureLayoutTransferDst);
			vkCmdCopyBufferToImage(cb->_priv->v, staging_buffer->_priv->v, t->_priv->v, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_regions.size(), buffer_copy_regions.data());
			t->change_layout(cb, TextureLayoutTransferDst, TextureLayoutShaderReadOnlyOptimal);
			cb->end();
			queue->submit(cb, nullptr, nullptr);
			queue->wait_idle();
			cp->destroy_commandbuffer(cb);
			destroy_buffer(d, staging_buffer);

			return t;
		}

		void destroy_texture(Device *d, Texture *t)
		{
			vkFreeMemory(d->_priv->device, t->_priv->m, nullptr);
			vkDestroyImage(d->_priv->device, t->_priv->v, nullptr);

			delete t->_priv;
			delete t;
		}

		Textureview *create_textureview(Device *d, Texture *t,
			TextureViewType view_type, int base_level, int level_count, int base_layer, int layer_count)
		{
			auto v = new Textureview;

			v->_priv = new TextureviewPrivate;

			auto ft = t->format.get_type();
			int aspect = 0;
			switch (ft)
			{
				case Format::TypeColor:
					aspect = TextureAspectColor;
					break;
				case Format::TypeDepth:
					aspect = TextureAspectDepth;
					break;
				case Format::TypeDepthStencil:
					aspect = TextureAspectDepth | TextureAspectStencil;
					break;
			}
			v->_priv->v = create_imageview(d, t->_priv->v, Z(t->format), Z(aspect), Z(view_type),
				base_level, level_count, base_layer, layer_count);

			return v;
		}

		void destroy_textureview(Device *d, Textureview *v)
		{
			destroy_imageview(d, v->_priv->v);

			delete v->_priv;
			delete v;
		}
	}
}

