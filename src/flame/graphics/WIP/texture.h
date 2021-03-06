#pragma once

#include <memory>

#include <flame/math.h>
//#include <flame/image.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{

	}

	//struct Buffer;
	//struct Texture;
	//struct CommandBuffer;

	//struct TextureLevel
	//{
	//	int cx;
	//	int cy;
	//	int pitch;
	//	int size_per_layer;
	//};

	//struct TextureView
	//{
	//	VkImageViewType view_type;
	//	int base_level;
	//	int level_count;
	//	int base_layer;
	//	int layer_count;
	//	VkImageView v;

	//	TextureView(VkImage i, VkFormat format, VkImageAspectFlags aspect, VkImageViewType _view_type = VK_IMAGE_VIEW_TYPE_2D, int _base_Level = 0, int _level_count = 1, int _base_layer = 0, int _layer_count = 1);
	//	~TextureView();
	//};

	//VkFormat get_texture_format(int bpp, int channel, bool sRGB);

	//enum TextureType
	//{
	//	TextureTypeAttachment,
	//	TextureTypeImage
	//};

	//struct Texture
	//{
	//	TextureType type;
	//	VkFormat format;
	//	std::vector<TextureLevel> levels;
	//	int layer_count;
	//	bool cube;

	//	int channel;
	//	int bpp;
	//	bool sRGB;
	//	int total_size;

	//	VkImageLayout layout;

	//	std::vector<std::unique_ptr<TextureView>> views;

	//	std::string filename;

	//	int material_index;
	//	int ui_index;
	//	int ui_ref_count;

	//	VkImage v;
	//	VkDeviceMemory memory;

	//	 must call in main thread
	//	Texture(TextureType _type, int _cx, int _cy, VkFormat _format, VkImageUsageFlags extra_usage, int _level = 1, int _layer = 1, bool _cube = false);
	//	 must call in main thread
	//	~Texture();
	//	VkImageAspectFlags get_aspect() const;
	//	int get_cx(int level = 0) const;
	//	int get_cy(int level = 0) const;
	//	int get_linear_offset(int x, int y, int level = 0, int layer = 0) const;
	//	VkImageView get_view(VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, int base_Level = 0, int level_count = 1, int base_layer = 0, int layer_count = 1);
	//	void transition_layout(CommandBuffer *cb, VkImageLayout old_layout, VkImageLayout new_layout, int base_level = 0, int level_count = 0, int base_layer = 0, int layer_count = 0);
	//private:
	//	void set_data_from_format();
	//};

	//std::shared_ptr<Texture> get_texture(const std::string &filename);

	//extern std::shared_ptr<Texture> default_color_texture; // R:0 G:0 B:0 A:0
	//extern std::shared_ptr<Texture> default_normal_texture; // X:0 Y:0 Z:1
	//extern std::shared_ptr<Texture> default_height_texture; // R:1 G:0 B:0 A:0
	//extern std::shared_ptr<Texture> default_blend_texture; // R:1 G:0 B:0 A:0

	//void init_texture();
}
