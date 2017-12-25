#pragma once

#include <memory>

#include "../math/math.h"
#include "graphics.h"
#include "../image_data.h"

namespace tke
{
	struct Image;
	struct ImageView
	{
		Image *image;
		VkImageAspectFlags aspect;
		int baseLevel;
		int levelCount;
		int baseLayer;
		int layerCount;
		VkImageView v;

		ImageView(Image *_image);
	};

	struct Image : ImageData
	{
		enum Type
		{
			eColor,
			eSwapchain,
			eDepth,
			eDepthStencil
		};
		Type type = eColor;
		inline bool isColorType() { return type == eColor || type == eSwapchain; }
		inline bool isDepthStencilType() { return type == eDepth || type == eDepthStencil; }

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImage v = 0;
		VkDeviceMemory memory = 0;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;

		std::vector<std::unique_ptr<ImageView>> views;

		std::vector<std::unique_ptr<VkDescriptorImageInfo>> infos;

		std::string filename;

		bool sRGB = false;

		int index = -1;

		// must call in main thread
		Image(int _cx, int _cy, VkFormat _format, VkImageUsageFlags usage, int _level = 1, int _layer = 1, bool needGeneralLayout = true);
		// must call in main thread
		Image(Type _type, VkImage _image, int _cx, int _cy, VkFormat _format);
		// must call in main thread
		~Image();
		void clear(const glm::vec4 &color);
		unsigned char getR(float x, float y);
		unsigned char getA(float x, float y);
		void transitionLayout(int _level, VkImageLayout _layout);
		void fillData(int _level, unsigned char *src, size_t _size);
		VkImageView getView(int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		VkDescriptorImageInfo *getInfo(VkImageView view, VkSampler sampler);
	};

	Image *load_image(const std::string &filename, int min_level = 0, bool sRGB = false, bool saveData = false);
	std::shared_ptr<Image> getImage(const std::string &filename, int min_level = 0, bool sRGB = false, bool saveData = false);
}
