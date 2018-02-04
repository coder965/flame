#include <assert.h>
#include <filesystem>
#include <map>

#include "../string_utils.h"
#include "../file_utils.h"
#include "buffer.h"
#include "image.h"
#include "command_buffer.h"

namespace tke
{
	Image::Image(int _cx, int _cy, VkFormat _format, VkImageUsageFlags usage, int _level, int _layer, bool need_general_layout) :
		format(_format),
		layout(VK_IMAGE_LAYOUT_UNDEFINED),
		view_type(VK_IMAGE_VIEW_TYPE_2D),
		sRGB(false),
		material_index(-1),
		ui_index(-1)
	{
		if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT)
		{
			type = TypeDepth;
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			type = TypeDepthStencil;
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			type = TypeColor;
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		assert(_level >= 1);
		assert(_layer >= 1);

		if (_level > 1)
			levels.resize(_level);

		auto cx = _cx;
		auto cy = _cy;
		for (int i = 0; i < _level; i++)
		{
			levels[i].cx = cx;
			levels[i].cy = cy;
			cx >>= 1; cx = glm::max(cx, 1);
			cy >>= 1; cy = glm::max(cy, 1);
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
		imageInfo.initialLayout = layout;
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

		total_size = memRequirements.size;

		if (need_general_layout)
		{
			for (int i = 0; i < levels.size(); i++)
				transition_layout(i, VK_IMAGE_LAYOUT_GENERAL);
		}
	}

	Image::Image(Type _type, VkImage _image, int _cx, int _cy, VkFormat _format) :
		type(_type),
		v(_image),
		memory(0),
		format(_format),
		layout(VK_IMAGE_LAYOUT_UNDEFINED),
		view_type(VK_IMAGE_VIEW_TYPE_2D),
		sRGB(false),
		material_index(-1),
		ui_index(-1)
	{
		aspect = type == TypeColor ? VK_IMAGE_ASPECT_COLOR_BIT : (type == TypeDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		levels[0].cx = _cx;
		levels[0].cy = _cy;
	}

	Image::~Image()
	{
		for (auto &v : views)
			vkDestroyImageView(vk_device.v, v->v, nullptr);
		if (memory)
		{
			vkFreeMemory(vk_device.v, memory, nullptr);
			vkDestroyImage(vk_device.v, v, nullptr);
		}
	}

	void Image::clear(const glm::vec4 &color)
	{
		auto cb = begineOnceCommandBuffer();
		VkClearColorValue clear_value = { color.x, color.y, color.z, color.a };
		VkImageSubresourceRange range = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, levels.size(),
			0, 1
		};
		vkCmdClearColorImage(cb->v, v, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &range);
		endOnceCommandBuffer(cb);
	}

	unsigned char Image::get_r(float _x, float _y)
	{
		if (!levels[0].v || _x < 0.f || _y < 0.f || _x >= levels[0].cx || _y >= levels[0].cy)
			return 0;

		auto x = glm::fract(_x);
		int X = glm::floor(_x);
		auto y = glm::fract(_y);
		int Y = glm::floor(_y);

		auto pixel_size = bpp / 8;
#define gd(a, b) (float)levels[0].v[(a) * pixel_size + (b) * levels[0].pitch]
		return glm::mix(glm::mix(gd(X, Y), gd(X + 1, Y), x), glm::mix(gd(X, Y + 1), gd(X + 1, Y + 1), x), y);
#undef gd
	}

	unsigned char Image::get_a(float _x, float _y)
	{
		if (!levels[0].v || _x < 0.f || _y < 0.f || _x >= levels[0].cx || _y >= levels[0].cy)
			return 0;

		auto x = glm::fract(_x);
		int X = glm::floor(_x);
		auto y = glm::fract(_y);
		int Y = glm::floor(_y);

		auto pixel_size = bpp / 8;
#define gd(a, b) (float)levels[0].v[(a) * pixel_size + 3 + (b) * levels[0].pitch]
		return glm::mix(glm::mix(gd(X, Y), gd(X + 1, Y), x), glm::mix(gd(X, Y + 1), gd(X + 1, Y + 1), x), y);
#undef gd
	}

	void Image::transition_layout(int _level, VkImageLayout _layout)
	{
		auto cb = begineOnceCommandBuffer();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = layout;
		barrier.newLayout = _layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = v;
		barrier.subresourceRange.aspectMask = aspect;
		barrier.subresourceRange.baseMipLevel = _level;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		switch (layout)
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

		endOnceCommandBuffer(cb);

		layout = _layout;
	}

	void Image::fill_data(int _level, unsigned char *src, size_t _size)
	{
		transition_layout(_level, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		StagingBuffer stagingBuffer(_size);

		void* map = stagingBuffer.map(0, _size);
		memcpy(map, src, _size);
		stagingBuffer.unmap();

		levels[_level].size = _size;

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = levels[_level].cx;
		region.imageExtent.height = levels[_level].cy;
		region.imageExtent.depth = 1;
		region.bufferOffset = 0;

		auto cb = begineOnceCommandBuffer();
		vkCmdCopyBufferToImage(cb->v, stagingBuffer.v, v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		endOnceCommandBuffer(cb);

		transition_layout(_level, VK_IMAGE_LAYOUT_GENERAL);
	}

	VkImageView Image::get_view(int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		for (auto &view : views)
		{
			if (view->baseLevel == baseLevel && view->levelCount == levelCount &&
				view->baseLayer == baseLayer && view->layerCount == layerCount)
				return view->v;
		}

		auto view = new ImageView;
		view->baseLevel = baseLevel;
		view->levelCount = levelCount;
		view->baseLayer = baseLayer;
		view->layerCount = layerCount;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = v;
		info.viewType = view_type;
		info.format = format;
		info.subresourceRange.aspectMask = aspect;
		info.subresourceRange.baseMipLevel = baseLevel;
		info.subresourceRange.levelCount = levelCount;
		info.subresourceRange.baseArrayLayer = baseLayer;
		info.subresourceRange.layerCount = layerCount;

		auto res = vkCreateImageView(vk_device.v, &info, nullptr, &view->v);
		assert(res == VK_SUCCESS);

		views.emplace_back(view);
		return view->v;
	}

	VkDescriptorImageInfo *Image::get_info(VkImageView view, VkSampler sampler)
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

	Image *load_image(const std::string &filename)
	{
		auto image_data = createImageData(filename);
		if (!image_data)
			return nullptr;

		bool sRGB = false;
		if (std::fs::exists(filename + ".srgb"))
			sRGB = true;
		sRGB = sRGB || image_data->sRGB;

		VkFormat _format = VK_FORMAT_UNDEFINED;
		switch (image_data->channel)
		{
			case 0:
				switch (image_data->bpp)
				{
					case 8:
						_format = VK_FORMAT_R8_UNORM;
						break;
				}
				break;
			case 1:
				switch (image_data->bpp)
				{
					case 8:
						_format = VK_FORMAT_R8_UNORM;
						break;
					case 16:
						_format = VK_FORMAT_R16_UNORM;
						break;
				}
				break;
			case 3:
				// vk do not support 3 channels
				break;
			case 4:
				switch (image_data->bpp)
				{
					case 32:
						if (sRGB)
							_format = VK_FORMAT_B8G8R8A8_SRGB/*VK_FORMAT_R8G8B8A8_SRGB*/;
						else
							_format = VK_FORMAT_B8G8R8A8_UNORM/*VK_FORMAT_R8G8B8A8_UNORM*/;
						break;
				}
		}
		assert(_format != VK_FORMAT_UNDEFINED);

		auto i = new Image(image_data->levels[0].cx, image_data->levels[0].cy,
			_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, image_data->levels.size(), 1, false);
		for (int l = 0; l < image_data->levels.size(); l++)
		{
			i->fill_data(l, image_data->levels[l].v.get(), image_data->levels[l].size);
			i->levels[l].pitch = image_data->levels[l].pitch;
		}
		i->filename = filename;
		i->bpp = image_data->bpp;
		i->sRGB = sRGB;

		return i;
	}

	static std::map<unsigned int, std::weak_ptr<Image>> _images;

	std::shared_ptr<Image> get_image(const std::string &filename)
	{
		auto hash = HASH(filename.c_str());
		auto it = _images.find(hash);
		if (it != _images.end())
		{
			auto s = it->second.lock();
			if (s)
				return s;
		}

		auto i = std::shared_ptr<Image>(load_image(filename));
		if (!i)
			return nullptr;

		_images[hash] = i;
		return i;
	}

	std::shared_ptr<Image> default_color_image;
	std::shared_ptr<Image> default_normal_image;
	std::shared_ptr<Image> default_blend_image;

	void init_image()
	{
		default_color_image = std::make_shared<Image>(4, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		default_color_image->filename = "[default_color_image]";
		default_color_image->clear(glm::vec4(0.f));
		default_normal_image = std::make_shared<Image>(4, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		default_normal_image->filename = "[default_normal_image]";
		default_normal_image->clear(glm::vec4(0.f, 0.f, 1.f, 0.f));
		default_blend_image = std::make_shared<Image>(4, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		default_blend_image->filename = "[default_blend_image]";
		default_blend_image->clear(glm::vec4(1.f, 0.f, 0.f, 0.f));
	}
}
