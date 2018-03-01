#include <assert.h>
#include <filesystem>
#include <map>

#include <gli/gli.hpp>
#include <flame/utils/string.h>
#include <flame/utils/filesystem.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/command_buffer.h>

namespace flame
{
	Texture::Texture(int _cx, int _cy, VkFormat _format, VkImageUsageFlags usage, int _level, int _layer, bool need_general_layout) :
		format(_format),
		view_type(VK_IMAGE_VIEW_TYPE_2D),
		layout(VK_IMAGE_LAYOUT_UNDEFINED),
		layer(1),
		sRGB(false),
		material_index(-1),
		ui_index(-1)
	{
		set_data_from_format();

		assert(_level >= 1);
		assert(_layer >= 1);

		auto cx = _cx;
		auto cy = _cy;
		levels.resize(_level);
		for (int i = 0; i < _level; i++)
		{
			levels[i] = std::make_unique<TextureLevel>();
			levels[i]->cx = cx;
			levels[i]->cy = cy;
			levels[i]->pitch = calc_pitch(cx * (bpp / 8));
			cx >>= 1;
			cy >>= 1;
			cx = glm::max(cx, 1);
			cy = glm::max(cy, 1);
		}

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = _cx;
		imageInfo.extent.height = _cy;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = _level;
		imageInfo.arrayLayers = _layer;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto res = vkCreateImage(vk_device.v, &imageInfo, nullptr, &v);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(vk_device.v, v, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = find_vk_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(vk_device.v, &allocInfo, nullptr, &memory);
		assert(res == VK_SUCCESS);

		res = vkBindImageMemory(vk_device.v, v, memory, 0);
		assert(res == VK_SUCCESS);

		//total_size = memRequirements.size;

		if (need_general_layout)
			transition_layout(VK_IMAGE_LAYOUT_GENERAL);
	}

	Texture::Texture(VkImage _image, int _cx, int _cy, VkFormat _format) :
		v(_image),
		memory(0),
		format(_format),
		view_type(VK_IMAGE_VIEW_TYPE_2D),
		layout(VK_IMAGE_LAYOUT_UNDEFINED),
		layer(1),
		sRGB(false),
		material_index(-1),
		ui_index(-1)
	{
		set_data_from_format();

		levels.resize(1);
		levels[0] = std::make_unique<TextureLevel>();
		levels[0]->cx = _cx;
		levels[0]->cy = _cy;
		levels[0]->pitch = calc_pitch(_cx * (bpp / 8));
	}

	Texture::~Texture()
	{
		for (auto &v : views)
			vkDestroyImageView(vk_device.v, v->v, nullptr);
		if (memory)
		{
			vkFreeMemory(vk_device.v, memory, nullptr);
			vkDestroyImage(vk_device.v, v, nullptr);
		}
	}

	VkImageAspectFlags Texture::get_aspect() const
	{
		switch (type)
		{
			case TypeColor:
				return VK_IMAGE_ASPECT_COLOR_BIT;
			case TypeDepth:
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			case TypeDepthStencil:
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		return 0;
	}

	int Texture::get_cx(int level) const
	{
		return levels[level]->cx;
	}

	int Texture::get_cy(int level) const
	{
		return levels[level]->cy;
	}

	int Texture::get_size(int level) const
	{
		auto l = levels[level].get();
		return l->pitch * l->cy;
	}

	int Texture::get_linear_offset(int x, int y, int level) const
	{
		auto l = levels[level].get();
		return l->pitch * y + x * (bpp / 8);
	}

	void Texture::clear(const glm::vec4 &color)
	{
		auto last_layout = layout;
		transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		auto cb = begin_once_command_buffer();
		VkClearColorValue clear_value = { color.x, color.y, color.z, color.a };
		VkImageSubresourceRange range = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, levels.size(),
			0, 1
		};
		vkCmdClearColorImage(cb->v, v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &range);
		end_once_command_buffer(cb);
		transition_layout(last_layout);
	}

	void Texture::transition_layout(int _level, VkImageLayout _layout)
	{
		auto cb = begin_once_command_buffer();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = layout;
		barrier.newLayout = _layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = v;
		barrier.subresourceRange.aspectMask = get_aspect();
		barrier.subresourceRange.baseMipLevel = _level;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		switch (barrier.oldLayout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				//barrier.srcAccessMask |= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_GENERAL:
				break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				break;
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				break;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				break;
		}

		switch (_layout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				break;
			case VK_IMAGE_LAYOUT_GENERAL:
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
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				break;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				break;
		}

		vkCmdPipelineBarrier(cb->v, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		end_once_command_buffer(cb);

	}

	void Texture::transition_layout(VkImageLayout _layout)
	{
		for (auto i = 0; i < levels.size(); i++)
			transition_layout(i, _layout);
		layout = _layout;
	}

	void Texture::fill_data(int level, unsigned char *src)
	{
		auto size = get_size(level);

		Buffer staging_buffer(BufferTypeStaging, size);

		void* map = staging_buffer.map(0, size);
		memcpy(map, src, size);
		staging_buffer.unmap();

		copy_from_buffer(&staging_buffer, level);
	}

	void Texture::copy_to_buffer(Buffer *dst, int level, int x, int y, int width, int height, int buffer_offset)
	{
		if (width == 0)
			width = get_cx(level);
		if (height == 0)
			height = get_cy(level);
		if (buffer_offset < 0)
			buffer_offset = get_linear_offset(x, y, level);

		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = get_aspect();
		region.imageSubresource.mipLevel = level;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset.x = x;
		region.imageOffset.y = y;
		region.imageOffset.z = 0;
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = 1;
		region.bufferOffset = buffer_offset;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		auto last_layout = layout;
		transition_layout(level, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		auto cb = begin_once_command_buffer();
		vkCmdCopyImageToBuffer(cb->v, v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->v, 1, &region);
		end_once_command_buffer(cb);

		transition_layout(level, last_layout);
		layout = last_layout;
	}

	void Texture::copy_from_buffer(Buffer *src, int level, int x, int y, int width, int height, int buffer_offset)
	{
		if (width == 0)
			width = get_cx(level);
		if (height == 0)
			height = get_cy(level);
		if (buffer_offset < 0)
			buffer_offset = get_linear_offset(x, y, level);

		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = get_aspect();
		region.imageSubresource.mipLevel = level;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset.x = x;
		region.imageOffset.y = y;
		region.imageOffset.z = 0;
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = 1;
		region.bufferOffset = buffer_offset;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		transition_layout(level, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		auto cb = begin_once_command_buffer();
		vkCmdCopyBufferToImage(cb->v, src->v, v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		end_once_command_buffer(cb);

		transition_layout(level, VK_IMAGE_LAYOUT_GENERAL);
	}

	VkImageView Texture::get_view(int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		for (auto &view : views)
		{
			if (view->baseLevel == baseLevel && view->levelCount == levelCount &&
				view->baseLayer == baseLayer && view->layerCount == layerCount)
				return view->v;
		}

		auto view = new TextureView;
		view->baseLevel = baseLevel;
		view->levelCount = levelCount;
		view->baseLayer = baseLayer;
		view->layerCount = layerCount;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = v;
		info.viewType = view_type;
		info.format = format;
		info.subresourceRange.aspectMask = get_aspect();
		info.subresourceRange.baseMipLevel = baseLevel;
		info.subresourceRange.levelCount = levelCount;
		info.subresourceRange.baseArrayLayer = baseLayer;
		info.subresourceRange.layerCount = layerCount;

		auto res = vkCreateImageView(vk_device.v, &info, nullptr, &view->v);
		assert(res == VK_SUCCESS);

		views.emplace_back(view);
		return view->v;
	}

	void Texture::set_data_from_format()
	{
		switch (format)
		{
			case VK_FORMAT_R8_UNORM:
				type = TypeColor;
				bpp = 8;
				break;
			case VK_FORMAT_R16_UNORM:
				type = TypeColor;
				bpp = 16;
				break;
			case VK_FORMAT_R8G8B8A8_UNORM:
				type = TypeColor;
				bpp = 32;
				break;
			case VK_FORMAT_R8G8B8A8_SRGB:
				type = TypeColor;
				bpp = 32;
				break;
			case VK_FORMAT_B8G8R8A8_UNORM:
				type = TypeColor;
				bpp = 32;
				break;
			case VK_FORMAT_B8G8R8A8_SRGB:
				type = TypeColor;
				bpp = 32;
				break;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				type = TypeColor;
				bpp = 64;
				break;
			case VK_FORMAT_R16G16B16A16_UNORM:
				type = TypeColor;
				bpp = 64;
				break;
			case VK_FORMAT_D16_UNORM:
				type = TypeDepth;
				bpp = 16;
				break;
			case VK_FORMAT_D32_SFLOAT:
				type = TypeDepth;
				bpp = 32;
				break;
			case VK_FORMAT_D16_UNORM_S8_UINT:
				type = TypeDepthStencil;
				bpp = 24;
				break;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				type = TypeDepthStencil;
				bpp = 32;
				break;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				type = TypeDepthStencil;
				bpp = 40;
				break;
		}
	}

	VkDescriptorImageInfo *Texture::get_info(VkImageView view, VkSampler sampler)
	{
		for (auto &i : infos)
		{
			if (i->imageView == view && i->sampler == sampler)
				return i.get();
		}
		auto i = new VkDescriptorImageInfo;
		i->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		i->imageView = view;
		i->sampler = sampler;
		infos.emplace_back(i);
		return i;
	}

	static VkFormat _get_texture_format(int bpp, int channel, bool sRGB)
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
							return VK_FORMAT_B8G8R8A8_SRGB/*VK_FORMAT_R8G8B8A8_SRGB*/;
						else
							return VK_FORMAT_B8G8R8A8_UNORM/*VK_FORMAT_R8G8B8A8_UNORM*/;
				}
		}
		return VK_FORMAT_UNDEFINED;
	}

	static std::map<unsigned int, std::weak_ptr<Texture>> _images;

	std::shared_ptr<Texture> get_or_create_texture(const std::string &filename)
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

		std::shared_ptr<Texture> t;

		auto ext = path.extension().string();
		if (ext == ".ktx" || ext == ".dds")
		{
			gli::texture _Texture = gli::load(filename);
			if (_Texture.empty())
				assert(0);

			gli::gl GL(gli::gl::PROFILE_GL33);
			auto const Format = GL.translate(_Texture.format(), _Texture.swizzles());
			auto Target = GL.translate(_Texture.target());
			assert(!gli::is_compressed(_Texture.format()) && Target == gli::TARGET_2D);
			assert(Format.External != gli::gl::EXTERNAL_NONE && Format.Type != gli::gl::TYPE_NONE);

			auto channel = 0;
			auto sRGB = false;
			switch (Format.External)
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
				case gli::gl::EXTERNAL_DEPTH:
					assert(0); // WIP
				case gli::gl::EXTERNAL_DEPTH_STENCIL:
					assert(0); // WIP
				case gli::gl::EXTERNAL_STENCIL:
					assert(0); // WIP
				case gli::gl::EXTERNAL_LUMINANCE:
					assert(0); // WIP
				case gli::gl::EXTERNAL_ALPHA:
					channel = 1;
					break;
				case gli::gl::EXTERNAL_LUMINANCE_ALPHA:
					assert(0); // WIP
				case gli::gl::EXTERNAL_SRGB_EXT:
					channel = 3;
					sRGB = true;
					break;
				case gli::gl::EXTERNAL_SRGB_ALPHA_EXT:
					channel = 4;
					sRGB = true;
					break;
			}
			auto bpp = 0;
			switch (Format.Type)
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
				case gli::gl::TYPE_F16_OES:
					assert(0); // WIP
				case gli::gl::TYPE_F32:
					bpp = 32;
					break;
				case gli::gl::TYPE_F64:
					bpp = 64;
					break;
				case gli::gl::TYPE_UINT32_RGB9_E5_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT32_RG11B10F_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT8_RG3B2:
					assert(0); // WIP
				case gli::gl::TYPE_UINT8_RG3B2_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGB5A1:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGB5A1_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_R5G6B5:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_R5G6B5_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGBA4:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGBA4_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT32_RGBA8:
					bpp = 8;
					break;
				case gli::gl::TYPE_UINT32_RGBA8_REV:
					bpp = 8;
					break;
				case gli::gl::TYPE_UINT32_RGB10A2:
					assert(0); // WIP
				case gli::gl::TYPE_UINT32_RGB10A2_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT8_RG4_REV_GTC:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_A1RGB5_GTC:
					assert(0); // WIP
			}
			auto _format = _get_texture_format(bpp, channel, sRGB);
			assert(_format != VK_FORMAT_UNDEFINED);

			t = std::make_shared<Texture>(_Texture.extent().x, _Texture.extent().y,
				_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, _Texture.levels(), 1, false);
			for (auto i = 0; i < _Texture.levels(); i++)
			{
				t->fill_data(i, (unsigned char*)_Texture.data(0, 0, i));
				t->levels[i]->pitch = calc_pitch(_Texture.extent(i).x, bpp);
			}
			t->bpp = bpp;
			t->sRGB = sRGB;
		}
		else
		{
			auto image = std::make_unique<Image>(filename);

			auto sRGB = std::filesystem::exists(filename + ".srgb") || image->sRGB;

			auto _format = _get_texture_format(image->bpp, image->channel, sRGB);
			assert(_format != VK_FORMAT_UNDEFINED);

			t = std::make_shared<Texture>(image->cx, image->cy,
				_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
			t->fill_data(0, image->data.get());
			t->levels[0]->pitch = calc_pitch(image->cx, image->bpp);
			t->bpp = image->bpp;
			t->sRGB = sRGB;
		}

		t->filename = filename;

		_images[hash] = t;
		return t;
	}

	std::shared_ptr<Texture> default_color_texture;
	std::shared_ptr<Texture> default_normal_texture;
	std::shared_ptr<Texture> default_blend_texture;

	void init_texture()
	{
		default_color_texture = std::make_shared<Texture>(4, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		default_color_texture->filename = "[default_color_texture]";
		default_color_texture->clear(glm::vec4(0.f));
		default_normal_texture = std::make_shared<Texture>(4, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		default_normal_texture->filename = "[default_normal_texture]";
		default_normal_texture->clear(glm::vec4(0.f, 0.f, 1.f, 0.f));
		default_blend_texture = std::make_shared<Texture>(4, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		default_blend_texture->filename = "[default_blend_texture]";
		default_blend_texture->clear(glm::vec4(1.f, 0.f, 0.f, 0.f));
	}
}
