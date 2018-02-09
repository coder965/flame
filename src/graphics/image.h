#pragma once

#include <memory>

#include "../math/math.h"
#include "../image_utils.h"
#include "graphics.h"

namespace tke
{
	struct Buffer;

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
		VkImageAspectFlags get_aspect() const;
		int get_cx(int level = 0) const;
		int get_cy(int level = 0) const;
		int get_size(int level = 0) const;
		int get_linear_offset(int x, int y, int level = 0) const;
		VkImageView get_view(int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		VkDescriptorImageInfo *get_info(VkImageView view, VkSampler sampler);
		void transition_layout(int _level, VkImageLayout _layout);
		void transition_layout(VkImageLayout _layout);
		void clear(const glm::vec4 &color);
		void fill_data(int level, unsigned char *src);
		void copy_to_buffer(Buffer *dst, int level = 0, int x = 0, int y = 0, int width = 0, int height = 0, int buffer_offset = 0);
		void copy_from_buffer(Buffer *src, int level = 0, int x = 0, int y = 0, int width = 0, int height = 0, int buffer_offset = 0);
	private:
		void set_data_from_format();
	};

	Image *load_image(const std::string &filename);
	std::shared_ptr<Image> get_image(const std::string &filename);

	extern std::shared_ptr<Image> default_color_image; // R:0 G:0 B:0 A:0
	extern std::shared_ptr<Image> default_normal_image; // X:0 Y:0 Z:1
	extern std::shared_ptr<Image> default_blend_image; // R:1 G:0 B:0 A:0

	void init_image();
}
