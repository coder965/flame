#pragma once

#include <memory>

#include "../math/math.h"
#include "../image_data.h"
#include "graphics.h"

namespace tke
{
	struct ImageLevel
	{
		unsigned int cx;
		unsigned int cy;
		unsigned int pitch;
		VkImageLayout layout;
	};

	struct ImageView
	{
		int baseLevel;
		int levelCount;
		int baseLayer;
		int layerCount;
		VkImageView v;
	};

	struct Image
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
		VkImageViewType view_type;

		unsigned int bpp;
		std::vector<std::unique_ptr<ImageLevel>> levels;
		unsigned int layer;
		bool sRGB;

		std::vector<std::unique_ptr<ImageView>> views;

		std::vector<std::unique_ptr<VkDescriptorImageInfo>> infos;

		std::string filename;

		int material_index;
		int ui_index;

		// must call in main thread
		Image(int _cx, int _cy, VkFormat _format, VkImageUsageFlags usage, int _level = 1, int _layer = 1, bool need_general_layout = true);
		// must call in main thread
		Image(VkImage _image, int _cx, int _cy, VkFormat _format);
		// must call in main thread
		~Image();
		int get_cx(int _level = 0) const;
		int get_cy(int _level = 0) const;
		void clear(const glm::vec4 &color);
		void transition_layout(int _level, VkImageLayout _layout);
		void transition_layout(VkImageLayout _layout);
		void fill_data(int _level, unsigned char *src, size_t _size);
		VkImageView get_view(int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		VkDescriptorImageInfo *get_info(VkImageView view, VkSampler sampler);
	private:
		void set_type_and_aspect_from_format();
	};

	Image *load_image(const std::string &filename);
	std::shared_ptr<Image> get_image(const std::string &filename);

	extern std::shared_ptr<Image> default_color_image; // R:0 G:0 B:0 A:0
	extern std::shared_ptr<Image> default_normal_image; // X:0 Y:0 Z:1
	extern std::shared_ptr<Image> default_blend_image; // R:1 G:0 B:0 A:0

	void init_image();
}
