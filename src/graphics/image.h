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

		int material_index = -1;

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

	Image *load_image(const std::string &filename);
	std::shared_ptr<Image> get_image(const std::string &filename);

	extern std::shared_ptr<Image> default_color_image; // R:0 G:0 B:0 A:0
	extern std::shared_ptr<Image> default_normal_image; // X:0 Y:0 Z:1
	extern std::shared_ptr<Image> default_blend_image; // R:1 G:0 B:0 A:0

	void init_image();
}
