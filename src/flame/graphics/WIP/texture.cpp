#include <assert.h>
#include <filesystem>
#include <map>
#include <gli/gli.hpp>

#include <flame/string.h>
#include <flame/filesystem.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/ui/ui.h>

namespace flame
{
	Texture::Texture(TextureType _type, int _cx, int _cy, VkFormat _format, VkImageUsageFlags extra_usage, int _level, int _layer, bool _cube) :
		type(_type),
		format(_format),
		layer_count(_layer),
		cube(_cube),
		sRGB(false),
		material_index(-1),
		ui_index(-1),
		ui_ref_count(0)
	{
	}

	Texture::~Texture()
	{
		if (ui_index != -1)
			ui::unregister_texture(this);
	}

	int Texture::get_cx(int level) const
	{
		return levels[level].cx;
	}

	int Texture::get_cy(int level) const
	{
		return levels[level].cy;
	}

	int Texture::get_linear_offset(int x, int y, int level, int layer) const
	{
		auto offset = 0;
		for (auto i = 0; i < level - 1; i++)
			offset += levels[i].size_per_layer * layer_count;
		offset += levels[level].size_per_layer * layer;
		return offset + levels[level].pitch * y + x * (bpp / 8);
	}

	VkImageView Texture::get_view(VkImageViewType view_type, int base_level, int level_count, int base_layer, int layer_count)
	{
		for (auto &view : views)
		{
			if (view->view_type == view_type && view->base_level == base_level && view->level_count == level_count &&
				view->base_layer == base_layer && view->layer_count == layer_count)
				return view->v;
		}

		auto view = new TextureView(v, format, get_aspect(), view_type, base_level, level_count, base_layer, layer_count);
		views.emplace_back(view);
		return view->v;
	}

	static std::map<unsigned int, std::weak_ptr<Texture>> _images;

	std::shared_ptr<Texture> get_texture(const std::string &filename)
	{
		auto hash = HASH(filename.c_str());
		auto it = _images.find(hash);
		if (it != _images.end())
		{
			auto s = it->second.lock();
			if (s)
				return s;
		}

		auto sRGB = false, cube = false;

		{
			std::ifstream ext(filename + ".ext");
			if (ext.good())
			{
				std::string line;
				while (!ext.eof())
				{
					std::getline(ext, line);
					if (line == "srgb")
						sRGB = true;
					else if (line == "cube")
						cube = true;
				}
			}
		}

		auto ext = path.extension().string();
		if (ext == ".ktx" || ext == ".dds")
		{
			if (cube)
			{
				gli::texture_cube gli_texture_cube(gli_texture);
				auto offset = 0;
				for (auto j = 0; j < 6; j++)
				{
					for (auto i = 0; i < level; i++)
					{
						VkBufferImageCopy r = {};
						r.bufferOffset = offset;
						r.imageExtent.width = gli_texture_cube[j][i].extent().x;
						r.imageExtent.height = gli_texture_cube[j][i].extent().y;
						r.imageExtent.depth = 1;
						r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						r.imageSubresource.mipLevel = i;
						r.imageSubresource.baseArrayLayer = j;
						r.imageSubresource.layerCount = 1;
						buffer_copy_regions.push_back(r);
						offset += gli_texture_cube[j][i].size();
					}
				}
			}
			else
			{
			}
		}

		auto format = get_texture_format(channel, bpp, sRGB);
		assert(format != VK_FORMAT_UNDEFINED);
		auto t = std::make_shared<Texture>(TextureTypeImage, width, height,
			format, 0, level, layer, cube);
		t->filename = filename;

		_images[hash] = t;
		return t;
	}

	std::shared_ptr<Texture> default_color_texture;
	std::shared_ptr<Texture> default_normal_texture;
	std::shared_ptr<Texture> default_height_texture;
	std::shared_ptr<Texture> default_blend_texture;

	void init_texture()
	{
		auto cb = begin_once_command_buffer();

		auto init_with_color = [&](Texture *t, const glm::vec4 &color){
			t->transition_layout(cb, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			VkClearColorValue clear_value = { color.x, color.y, color.z, color.a };
			VkImageSubresourceRange range = {
				VK_IMAGE_ASPECT_COLOR_BIT,
				0, t->levels.size(),
				0, t->layer_count
			};
			vkCmdClearColorImage(cb->v, t->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &range);
			t->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		};

		default_color_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8G8B8A8_UNORM, 0);
		default_color_texture->filename = "[default_color_texture]";
		init_with_color(default_color_texture.get(), glm::vec4(0.f));
		default_normal_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8G8B8A8_UNORM, 0);
		default_normal_texture->filename = "[default_normal_texture]";
		init_with_color(default_normal_texture.get(), glm::vec4(0.f, 0.f, 1.f, 0.f));
		default_blend_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8G8B8A8_UNORM, 0);
		default_blend_texture->filename = "[default_blend_texture]";
		init_with_color(default_blend_texture.get(), glm::vec4(1.f, 0.f, 0.f, 0.f));
		default_height_texture = std::make_shared<Texture>(TextureTypeImage, 4, 4, VK_FORMAT_R8_UNORM, 0);
		default_height_texture->filename = "[default_height_texture]";
		init_with_color(default_height_texture.get(), glm::vec4(1.f, 0.f, 0.f, 0.f));

		end_once_command_buffer(cb);
	}
}
