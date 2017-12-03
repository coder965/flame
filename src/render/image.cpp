#include <assert.h>
#include <filesystem>
#include <map>

#include "../utils.h"
#include "../math/math.h"
#include "../core.h"
#include "buffer.h"
#include "image.h"
#include "command_buffer.h"

namespace tke
{
	ImageView::ImageView(Image *_image)
		:image(_image)
	{
	}

	Image::Image(int _cx, int _cy, VkFormat _format, VkImageUsageFlags usage, int _level, int _layer, bool needGeneralLayout)
		: format(_format)
	{
		if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT)
			type = eDepth;
		else if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
			type = eDepthStencil;
		else
			type = eColor;

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
		allocInfo.memoryTypeIndex = findVkMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(vk_device.v, &allocInfo, nullptr, &memory);
		assert(res == VK_SUCCESS);

		res = vkBindImageMemory(vk_device.v, v, memory, 0);
		assert(res == VK_SUCCESS);

		total_size = memRequirements.size;

		if (needGeneralLayout)
		{
			for (int i = 0; i < levels.size(); i++)
				transitionLayout(i, VK_IMAGE_LAYOUT_GENERAL);
		}
	}

	Image::Image(Type _type, VkImage _image, int _cx, int _cy, VkFormat _format)
	{
		type = _type;
		v = _image;
		levels[0].cx = _cx;
		levels[0].cy = _cy;
		format = _format;
	}

	Image::~Image()
	{
		for (auto &v : views)
			vkDestroyImageView(vk_device.v, v->v, nullptr);
		if (type != Type::eSwapchain)
		{
			vkFreeMemory(vk_device.v, memory, nullptr);
			vkDestroyImage(vk_device.v, v, nullptr);
		}
	}

	static VkImageAspectFlags _getImageAspect(Image *i)
	{
		if (i->isColorType())
			return VK_IMAGE_ASPECT_COLOR_BIT;
		if (i->type == Image::eDepth)
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	void Image::transitionLayout(int _level, VkImageLayout _layout)
	{
		auto cb = begineOnceCommandBuffer();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = layout;
		barrier.newLayout = _layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = v;
		barrier.subresourceRange.aspectMask = _getImageAspect(this);
		barrier.subresourceRange.baseMipLevel = _level;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(cb->v, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		endOnceCommandBuffer(cb);

		layout = _layout;
	}

	void Image::fillData(int _level, unsigned char *src, size_t _size)
	{
		transitionLayout(_level, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		StagingBuffer stagingBuffer(_size);

		void* map = stagingBuffer.map(0, _size);
		memcpy(map, src, _size);
		stagingBuffer.unmap();

		levels[_level].size = _size;

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = _getImageAspect(this);
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

		transitionLayout(_level, VK_IMAGE_LAYOUT_GENERAL);
	}

	VkImageView Image::getView(int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		auto aspect = _getImageAspect(this);

		for (auto &view : views)
		{
			if (view->aspect == aspect && view->baseLevel == baseLevel && view->levelCount == levelCount &&
				view->baseLayer == baseLayer && view->layerCount == layerCount)
				return view->v;
		}

		auto view = new ImageView(this);
		view->aspect = aspect;
		view->baseLevel = baseLevel;
		view->levelCount = levelCount;
		view->baseLayer = baseLayer;
		view->layerCount = layerCount;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = v;
		info.viewType = viewType;
		info.format = format;
		info.subresourceRange.aspectMask = aspect;
		info.subresourceRange.baseMipLevel = baseLevel;
		info.subresourceRange.levelCount = levelCount;
		info.subresourceRange.baseArrayLayer = baseLayer;
		info.subresourceRange.layerCount = layerCount;

		auto res = vkCreateImageView(vk_device.v, &info, nullptr, &view->v);
		assert(res == VK_SUCCESS);

		views.push_back(std::move(std::unique_ptr<ImageView>(view)));
		return view->v;
	}

	VkDescriptorImageInfo *Image::getInfo(VkImageView view, VkSampler sampler)
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
		infos.push_back(std::move(std::unique_ptr<VkDescriptorImageInfo>(i)));
		return i;
	}

	unsigned char Image::getR(float _x, float _y)
	{
		if (!levels[0].v || _x < 0.f || _y < 0.f || _x >= levels[0].cx || _y >= levels[0].cy)
			return 0;

		auto x = glm::fract(_x);
		int X = glm::floor(_x);
		auto y = glm::fract(_y);
		int Y = glm::floor(_y);

#define gd(a, b) (float)levels[0].v[(a) * byte_per_pixel + (b) * levels[0].pitch]
		return glm::mix(glm::mix(gd(X, Y), gd(X + 1, Y), x), glm::mix(gd(X, Y + 1), gd(X + 1, Y + 1), x), y);
#undef gd
	}

	static std::map<unsigned int, std::weak_ptr<Image>> _images;

	std::shared_ptr<Image> getImage(const std::string &filename, bool sRGB, bool saveData)
	{
		auto hash = HASH(filename.c_str());
		auto it = _images.find(hash);
		if (it != _images.end())
		{
			auto s = it->second.lock();
			if (s) return s;
		}

		auto image_data = createImageData(filename);
		if (!image_data)
			return nullptr;

		VkFormat _format = VK_FORMAT_UNDEFINED;
		switch (image_data->channel)
		{
		case 1:
			switch (image_data->byte_per_pixel)
			{
			case 1:
				_format = VK_FORMAT_R8_UNORM;
				break;
			case 2:
				_format = VK_FORMAT_R16_UNORM;
				break;
			}
			break;
		case 4:
			switch (image_data->byte_per_pixel)
			{
			case 4:
				if (sRGB)
					_format = VK_FORMAT_B8G8R8A8_SRGB/*VK_FORMAT_R8G8B8A8_SRGB*/;
				else
					_format = VK_FORMAT_B8G8R8A8_UNORM/*VK_FORMAT_R8G8B8A8_UNORM*/;
				break;
			}
		}
		assert(_format != VK_FORMAT_UNDEFINED);

		auto i = std::make_shared<Image>(image_data->levels[0].cx, image_data->levels[0].cy, 
			_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, image_data->levels.size(), 1, false);
		for (int l = 0; l < image_data->levels.size(); l++)
		{
			i->fillData(l, image_data->levels[l].v.get(), image_data->levels[l].size);
			i->levels[l].pitch = image_data->levels[l].pitch;
		}
		i->filename = filename;
		i->byte_per_pixel = image_data->byte_per_pixel;
		i->sRGB = sRGB || image_data->sRGB;

		if (saveData)
		{
			for (int l = 0; l < image_data->levels.size(); l++)
			{
				i->levels[l].v = std::move(image_data->levels[l].v);
				image_data->levels[l].v = nullptr;
			}
		}

		_images[hash] = i;
		return i;
	}
}
