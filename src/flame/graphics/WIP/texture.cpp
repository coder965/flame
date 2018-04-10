#include <assert.h>
#include <filesystem>
#include <map>
#include <gli/gli.hpp>

#include <flame/string.h>
#include <flame/filesystem.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/ui/ui.h>

namespace flame
{
	VkFormat get_texture_format(int channel, int bpp, bool sRGB)
	{
		switch (channel)
		{
			case 0:
				switch (bpp)
				{
					case 8:
						return VK_FORMAT_R8_UNORM;
				}
				break;
			case 1:
				switch (bpp)
				{
					case 8:
						return VK_FORMAT_R8_UNORM;
					case 16:
						return VK_FORMAT_R16_UNORM;
				}
				break;
			case 3:
				// vk do not support 3 channels
				break;
			case 4:
				switch (bpp)
				{
					case 32:
						if (sRGB)
							return /*VK_FORMAT_B8G8R8A8_SRGB*/VK_FORMAT_R8G8B8A8_SRGB;
						else
							return /*VK_FORMAT_B8G8R8A8_UNORM*/VK_FORMAT_R8G8B8A8_UNORM;
				}
		}
		return VK_FORMAT_UNDEFINED;
	}

	Texture::Texture(TextureType _type, int _cx, int _cy, VkFormat _format, VkImageUsageFlags extra_usage, int _level, int _layer, bool _cube) :
		type(_type),
		format(_format),
		layer_count(_layer),
		cube(_cube),
		sRGB(false),
		material_index(-1),
		ui_index(-1),
		ui_ref_count(0)
	{
		set_data_from_format();

		assert(_level >= 1);
		assert(_layer >= 1);

		if (cube)
			assert(layer_count >= 6);

		auto cx = _cx;
		auto cy = _cy;
		levels.resize(_level);
		total_size = 0;
		for (int i = 0; i < _level; i++)
		{
			levels[i].cx = cx;
			levels[i].cy = cy;
			levels[i].pitch = calc_pitch(cx * (bpp / 8));
			levels[i].size_per_layer = levels[i].pitch * cy;
			cx >>= 1;
			cy >>= 1;
			cx = glm::max(cx, 1);
			cy = glm::max(cy, 1);
			total_size += levels[i].size_per_layer * layer_count;
		}

		auto usage = extra_usage;
		auto format_type = get_format_type(format);
		switch (type)
		{
			case TextureTypeAttachment:
				if (format_type == FormatTypeColor)
					usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				else
					usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				break;
			case TextureTypeImage:
				usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				break;
		}

		VkImageCreateInfo imageInfo;
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.flags = 0;
		imageInfo.pNext = nullptr;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = _cx;
		imageInfo.extent.height = _cy;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = levels.size();
		imageInfo.arrayLayers = layer_count;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0;
		imageInfo.pQueueFamilyIndices = nullptr;

		vk_chk_res(vkCreateImage(vk_device, &imageInfo, nullptr, &v));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(vk_device, v, &memRequirements);

		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = vk_find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vk_chk_res(vkAllocateMemory(vk_device, &allocInfo, nullptr, &memory));

		vk_chk_res(vkBindImageMemory(vk_device, v, memory, 0));

		if (type == TextureTypeImage)
			layout = VK_IMAGE_LAYOUT_UNDEFINED;
		else
		{
			layout = format_type == FormatTypeColor ?
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			auto cb = begin_once_command_buffer();
			transition_layout(cb, VK_IMAGE_LAYOUT_UNDEFINED, layout);
			end_once_command_buffer(cb);
		}
	}

	Texture::~Texture()
	{
		if (memory)
		{
			vkFreeMemory(vk_device, memory, nullptr);
			vkDestroyImage(vk_device, v, nullptr);
		}
		if (ui_index != -1)
			ui::unregister_texture(this);
	}

	VkImageAspectFlags Texture::get_aspect() const
	{
		switch (get_format_type(format))
		{
			case FormatTypeColor:
				return VK_IMAGE_ASPECT_COLOR_BIT;
			case FormatTypeDepth:
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			case FormatTypeDepthStencil:
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		return 0;
	}

	int Texture::get_cx(int level) const
	{
		return levels[level].cx;
	}

	int Texture::get_cy(int level) const
	{
		return levels[level].cy;
	}

	int Texture::get_linear_offset(int x, int y, int level, int layer) const
	{
		auto offset = 0;
		for (auto i = 0; i < level - 1; i++)
			offset += levels[i].size_per_layer * layer_count;
		offset += levels[level].size_per_layer * layer;
		return offset + levels[level].pitch * y + x * (bpp / 8);
	}

	VkImageView Texture::get_view(VkImageViewType view_type, int base_level, int level_count, int base_layer, int layer_count)
	{
		for (auto &view : views)
		{
			if (view->view_type == view_type && view->base_level == base_level && view->level_count == level_count &&
				view->base_layer == base_layer && view->layer_count == layer_count)
				return view->v;
		}

		auto view = new TextureView(v, format, get_aspect(), view_type, base_level, level_count, base_layer, layer_count);
		views.emplace_back(view);
		return view->v;
	}

	void Texture::transition_layout(CommandBuffer *cb, VkImageLayout old_layout, VkImageLayout new_layout, int base_level, int level_count, int base_layer, int _layer_count)
	{
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = v;
		barrier.subresourceRange.aspectMask = get_aspect();
		barrier.subresourceRange.baseMipLevel = base_level;
		barrier.subresourceRange.levelCount = level_count == 0 ? levels.size() : level_count;
		barrier.subresourceRange.baseArrayLayer = base_layer;
		barrier.subresourceRange.layerCount = _layer_count == 0 ? layer_count : _layer_count;

		switch (old_layout)
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

		switch (new_layout)
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

		vkCmdPipelineBarrier(cb->v, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void Texture::set_data_from_format()
	{
		switch (format)
		{
			case VK_FORMAT_R8_UNORM:
				channel = 1;
				bpp = 8;
				break;
			case VK_FORMAT_R16_UNORM:
				channel = 1;
				bpp = 16;
				break;
			case VK_FORMAT_R8G8B8A8_UNORM:
				channel = 4;
				bpp = 32;
				break;
			case VK_FORMAT_R8G8B8A8_SRGB:
				channel = 4;
				bpp = 32;
				sRGB = true;
				break;
			case VK_FORMAT_B8G8R8A8_UNORM:
				channel = 4;
				bpp = 32;
				break;
			case VK_FORMAT_B8G8R8A8_SRGB:
				channel = 4;
				bpp = 32;
				break;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				channel = 4;
				bpp = 64;
				break;
			case VK_FORMAT_R16G16B16A16_UNORM:
				channel = 4;
				bpp = 64;
				break;
			case VK_FORMAT_D16_UNORM:
				channel = 1;
				bpp = 16;
				break;
			case VK_FORMAT_D32_SFLOAT:
				channel = 1;
				bpp = 32;
				break;
			case VK_FORMAT_D16_UNORM_S8_UINT:
				channel = 1;
				bpp = 24;
				break;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				channel = 1;
				bpp = 32;
				break;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				channel = 1;
				bpp = 40;
				break;
		}
	}

	static std::map<unsigned int, std::weak_ptr<Texture>> _images;

	static void _gli_get_channel_bpp(const gli::gl::format &format, int &channel, int &bpp, bool &sRGB)
	{
		assert(format.External != gli::gl::EXTERNAL_NONE && format.Type != gli::gl::TYPE_NONE);
		switch (format.External)
		{
			case gli::gl::EXTERNAL_RED:
				channel = 1;
				break;
			case gli::gl::EXTERNAL_RG:
				channel = 2;
				break;
			case gli::gl::EXTERNAL_RGB:
				channel = 3;
				break;
			case gli::gl::EXTERNAL_BGR:
				channel = 3;
				break;
			case gli::gl::EXTERNAL_RGBA:
				channel = 4;
				break;
			case gli::gl::EXTERNAL_BGRA:
				channel = 4;
				break;
			case gli::gl::EXTERNAL_RED_INTEGER:
				channel = 1;
				break;
			case gli::gl::EXTERNAL_RG_INTEGER:
				channel = 2;
				break;
			case gli::gl::EXTERNAL_RGB_INTEGER:
				channel = 3;
				break;
			case gli::gl::EXTERNAL_BGR_INTEGER:
				channel = 3;
				break;
			case gli::gl::EXTERNAL_RGBA_INTEGER:
				channel = 4;
				break;
			case gli::gl::EXTERNAL_BGRA_INTEGER:
				channel = 4;
				break;
			case gli::gl::EXTERNAL_ALPHA:
				channel = 1;
				break;
			case gli::gl::EXTERNAL_SRGB_EXT:
				channel = 3;
				sRGB = true;
				break;
			case gli::gl::EXTERNAL_SRGB_ALPHA_EXT:
				channel = 4;
				sRGB = true;
				break;
			default:
				assert(0); // WIP
		}
		switch (format.Type)
		{
			case gli::gl::TYPE_I8:
				bpp = 8;
				break;
			case gli::gl::TYPE_U8:
				bpp = 8;
				break;
			case gli::gl::TYPE_I16:
				bpp = 16;
				break;
			case gli::gl::TYPE_U16:
				bpp = 16;
				break;
			case gli::gl::TYPE_I32:
				bpp = 32;
				break;
			case gli::gl::TYPE_U32:
				bpp = 32;
				break;
			case gli::gl::TYPE_I64:
				bpp = 64;
				break;
			case gli::gl::TYPE_F16:
				bpp = 16;
				break;
			case gli::gl::TYPE_F32:
				bpp = 32;
				break;
			case gli::gl::TYPE_F64:
				bpp = 64;
				break;
			case gli::gl::TYPE_UINT32_RGBA8:
				bpp = 8;
				break;
			case gli::gl::TYPE_UINT32_RGBA8_REV:
				bpp = 8;
				break;
			default:
				assert(0); // WIP
		}
	}

	std::shared_ptr<Texture> get_texture(const std::string &filename)
	{
		auto hash = HASH(filename.c_str());
		auto it = _images.find(hash);
		if (it != _images.end())
		{
			auto s = it->second.lock();
			if (s)
				return s;
		}

		std::filesystem::path path(filename);
		if (!std::filesystem::exists(path))
			return nullptr;

		auto sRGB = false, cube = false;

		{
			std::ifstream ext(filename + ".ext");
			if (ext.good())
			{
				std::string line;
				while (!ext.eof())
				{
					std::getline(ext, line);
					if (line == "srgb")
						sRGB = true;
					else if (line == "cube")
						cube = true;
				}
			}
		}

		int width, height, level, layer;
		int channel, bpp;
		std::unique_ptr<Buffer> staging_buffer;
		std::vector<VkBufferImageCopy> buffer_copy_regions;

		auto ext = path.extension().string();
		if (ext == ".ktx" || ext == ".dds")
		{
			gli::gl GL(gli::gl::PROFILE_GL33);

			auto gli_texture = gli::load(filename);
			if (gli_texture.empty())
				assert(0);

			auto const gli_format = GL.translate(gli_texture.format(), gli_texture.swizzles());
			assert(!gli::is_compressed(gli_texture.format()) && GL.translate(gli_texture.target()) == gli::TARGET_2D);

			width = gli_texture.extent().x;
			height = gli_texture.extent().y;
			level = gli_texture.levels();
			layer = cube ? 6 : gli_texture.layers();

			int channel, bpp;
			_gli_get_channel_bpp(gli_format, channel, bpp, sRGB);

			staging_buffer = std::make_unique<Buffer>(BufferTypeStaging, gli_texture.size());
			staging_buffer->map();
			memcpy(staging_buffer->mapped, gli_texture.data(), staging_buffer->size);
			staging_buffer->unmap();
			if (cube)
			{
				gli::texture_cube gli_texture_cube(gli_texture);
				auto offset = 0;
				for (auto j = 0; j < 6; j++)
				{
					for (auto i = 0; i < level; i++)
					{
						VkBufferImageCopy r = {};
						r.bufferOffset = offset;
						r.imageExtent.width = gli_texture_cube[j][i].extent().x;
						r.imageExtent.height = gli_texture_cube[j][i].extent().y;
						r.imageExtent.depth = 1;
						r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						r.imageSubresource.mipLevel = i;
						r.imageSubresource.baseArrayLayer = j;
						r.imageSubresource.layerCount = 1;
						buffer_copy_regions.push_back(r);
						offset += gli_texture_cube[j][i].size();
					}
				}
			}
			else
			{
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
		}
		else
		{
			auto image = load_image(filename);
			if (image->channel == 3)
				image->add_alpha_channel();

			width = image->cx;
			height = image->cy;
			level = layer = 1;
			channel = image->channel;
			bpp = image->bpp;

			staging_buffer = std::make_unique<Buffer>(BufferTypeStaging, image->size);
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

		auto format = get_texture_format(channel, bpp, sRGB);
		assert(format != VK_FORMAT_UNDEFINED);
		auto t = std::make_shared<Texture>(TextureTypeImage, width, height,
			format, 0, level, layer, cube);
		t->filename = filename;

		auto cb = begin_once_command_buffer();
		t->transition_layout(cb, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkCmdCopyBufferToImage(cb->v, staging_buffer->v, t->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_regions.size(), buffer_copy_regions.data());
		t->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		t->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		end_once_command_buffer(cb);

		_images[hash] = t;
		return t;
	}

	std::shared_ptr<Texture> default_color_texture;
	std::shared_ptr<Texture> default_normal_texture;
	std::shared_ptr<Texture> default_height_texture;
	std::shared_ptr<Texture> default_blend_texture;

	void init_texture()
	{
		auto cb = begin_once_command_buffer();

		auto init_with_color = [&](Texture *t, const glm::vec4 &color){
			t->transition_layout(cb, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			VkClearColorValue clear_value = { color.x, color.y, color.z, color.a };
			VkImageSubresourceRange range = {
				VK_IMAGE_ASPECT_COLOR_BIT,
				0, t->levels.size(),
				0, t->layer_count
			};
			vkCmdClearColorImage(cb->v, t->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &range);
			t->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		};

		default_color_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8G8B8A8_UNORM, 0);
		default_color_texture->filename = "[default_color_texture]";
		init_with_color(default_color_texture.get(), glm::vec4(0.f));
		default_normal_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8G8B8A8_UNORM, 0);
		default_normal_texture->filename = "[default_normal_texture]";
		init_with_color(default_normal_texture.get(), glm::vec4(0.f, 0.f, 1.f, 0.f));
		default_blend_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8G8B8A8_UNORM, 0);
		default_blend_texture->filename = "[default_blend_texture]";
		init_with_color(default_blend_texture.get(), glm::vec4(1.f, 0.f, 0.f, 0.f));
		default_height_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8_UNORM, 0);
		default_height_texture->filename = "[default_height_texture]";
		init_with_color(default_height_texture.get(), glm::vec4(1.f, 0.f, 0.f, 0.f));

		end_once_command_buffer(cb);
	}
}
