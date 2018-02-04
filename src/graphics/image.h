#pragma once

#include <memory>

#include "../math/math.h"
#include "graphics.h"
#include "../image_data.h"

namespace tke
{
	struct ImageView
	{
		int baseLevel;
		int levelCount;
		int baseLayer;
		int layerCount;
		VkImageView v;
	};

	struct Image : ImageData
	{
		enum Type
		{
			TypeColor,
			TypeDepth,
			TypeDepthStencil
		};

		Type type;
		VkImageAspectFlags aspect;

		VkFormat format;
		VkImage v;
		VkDeviceMemory memory;
		VkImageLayout layout;
		VkImageViewType view_type;

		std::vector<std::unique_ptr<ImageView>> views;

		std::vector<std::unique_ptr<VkDescriptorImageInfo>> infos;

		std::string filename;

		bool sRGB;

		int material_index;
		int ui_index;

		// must call in main thread
		Image(int _cx, int _cy, VkFormat _format, VkImageUsageFlags usage, int _level = 1, int _layer = 1, bool need_general_layout = true);
		// must call in main thread
		Image(Type _type, VkImage _image, int _cx, int _cy, VkFormat _format);
		// must call in main thread
		~Image();
		void clear(const glm::vec4 &color);
		unsigned char get_r(float x, float y);
		unsigned char get_a(float x, float y);
		void transition_layout(int _level, VkImageLayout _layout);
		void fill_data(int _level, unsigned char *src, size_t _size);
		VkImageView get_view(int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		VkDescriptorImageInfo *get_info(VkImageView view, VkSampler sampler);
	};

	Image *load_image(const std::string &filename);
	std::shared_ptr<Image> get_image(const std::string &filename);

	extern std::shared_ptr<Image> default_color_image; // R:0 G:0 B:0 A:0
	extern std::shared_ptr<Image> default_normal_image; // X:0 Y:0 Z:1
	extern std::shared_ptr<Image> default_blend_image; // R:1 G:0 B:0 A:0

	void init_image();
}
