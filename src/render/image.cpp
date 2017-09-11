#include <assert.h>
#include <filesystem>

#include "../math/math.h"
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

		VkResult res;

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
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		device.mtx.lock();

		res = vkCreateImage(device.v, &imageInfo, nullptr, &v);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.v, v, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findVkMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(device.v, &allocInfo, nullptr, &memory);
		assert(res == VK_SUCCESS);

		res = vkBindImageMemory(device.v, v, memory, 0);
		assert(res == VK_SUCCESS);

		device.mtx.unlock();

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
		device.mtx.lock();
		for (auto v : views)
			vkDestroyImageView(device.v, v->v, nullptr);
		if (type != Type::eSwapchain)
		{
			vkFreeMemory(device.v, memory, nullptr);
			vkDestroyImage(device.v, v, nullptr);
		}
		device.mtx.unlock();
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

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		if (layout == VK_IMAGE_LAYOUT_PREINITIALIZED) barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		else if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		if (layout == VK_IMAGE_LAYOUT_PREINITIALIZED) srcStage = VK_PIPELINE_STAGE_HOST_BIT;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		else if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		if (_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		else if (_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		else if (_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		else if (_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(cb->v, srcStage, _layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

		for (auto view : views)
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

		device.mtx.lock();
		auto res = vkCreateImageView(device.v, &info, nullptr, &view->v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		views.push_back(view);
		return view->v;
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

	Image *createImage(const std::string &filename, bool sRGB, bool saveData)
	{
		std::unique_ptr<ImageData> d(std::move(createImageData(filename)));
		assert(d.get());

		VkFormat _format = VK_FORMAT_UNDEFINED;
		switch (d->channel)
		{
		case 1:
			switch (d->byte_per_pixel)
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
			switch (d->byte_per_pixel)
			{
			case 4:
				if (sRGB)
					_format = VK_FORMAT_R8G8B8A8_SRGB;
				else
					_format = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			}
		}
		assert(_format != VK_FORMAT_UNDEFINED);

		auto i = new Image(d->levels[0].cx, d->levels[0].cy, _format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, d->levels.size(), 1, false);
		for (int l = 0; l < d->levels.size(); l++)
		{
			i->fillData(l, d->levels[l].v, d->levels[l].size);
			i->levels[l].pitch = d->levels[l].pitch;
		}
		i->full_filename = filename;
		std::experimental::filesystem::path path(filename);
		i->filename = path.filename().string();
		i->byte_per_pixel = d->byte_per_pixel;
		i->sRGB = sRGB || d->sRGB;

		if (saveData)
		{
			for (int l = 0; l < d->levels.size(); l++)
			{
				i->levels[l].v = d->levels[l].v;
				d->levels[l].v = nullptr;
			}
		}

		return i;
	}
}
