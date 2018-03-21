#pragma once

#include <memory>

#include <flame/common/math.h>
#include <flame/common/image.h>
#include <flame/engine/graphics/graphics.h>

namespace flame
{
	struct Buffer;

	struct TextureLevel
	{
		int cx;
		int cy;
		int pitch;
	};

	struct TextureView
	{
		VkImageViewType view_type;
		int baseLevel;
		int levelCount;
		int baseLayer;
		int layerCount;
		VkImageView v;
	};

	VkFormat get_texture_format(int bpp, int channel, bool sRGB);

	enum TextureType
	{
		TextureTypeAttachment,
		TextureTypeFile,
		TextureTypeTransfer
	};

	struct Texture
	{
		VkFormat format;
		VkImage v;
		VkDeviceMemory memory;
		VkImageLayout layout;

		int channel;
		int bpp;
		std::vector<std::unique_ptr<TextureLevel>> levels;
		int layer;
		bool sRGB;

		std::vector<std::unique_ptr<TextureView>> views;

		std::string filename;

		int material_index;
		int ui_index;

		// must call in main thread
		Texture(int _cx, int _cy, VkFormat _format, VkImageUsageFlags usage, int _level = 1, int _layer = 1, bool need_general_layout = true);
		// must call in main thread
		Texture(VkImage _image, int _cx, int _cy, VkFormat _format); // for swapchain
		// must call in main thread
		~Texture();
		VkImageAspectFlags get_aspect() const;
		int get_cx(int level = 0) const;
		int get_cy(int level = 0) const;
		int get_size(int level = 0) const;
		int get_linear_offset(int x, int y, int level = 0) const;
		VkImageView get_view(VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		void transition_layout(int _level, VkImageLayout _layout);
		void transition_layout(VkImageLayout _layout);
		void clear(const glm::vec4 &color);
		void fill_data(int level, unsigned char *src);
		void copy_to_buffer(Buffer *dst, int level = 0, int x = 0, int y = 0, int width = 0, int height = 0, int buffer_offset = 0);
		void copy_from_buffer(Buffer *src, int level = 0, int x = 0, int y = 0, int width = 0, int height = 0, int buffer_offset = 0);
	private:
		void set_data_from_format();
	};

	std::shared_ptr<Texture> get_texture(const std::string &filename);

	extern std::shared_ptr<Texture> default_color_texture; // R:0 G:0 B:0 A:0
	extern std::shared_ptr<Texture> default_normal_texture; // X:0 Y:0 Z:1
	extern std::shared_ptr<Texture> default_height_texture; // R:1 G:0 B:0 A:0
	extern std::shared_ptr<Texture> default_blend_texture; // R:1 G:0 B:0 A:0

	void init_texture();
}
