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
		set_data_from_format();

		assert(_level >= 1);
		assert(_layer >= 1);

		if (cube)
			assert(layer_count >= 6);

		auto cx = _cx;
		auto cy = _cy;
		levels.resize(_level);
		total_size = 0;
		for (int i = 0; i < _level; i++)
		{
			levels[i].cx = cx;
			levels[i].cy = cy;
			levels[i].pitch = calc_pitch(cx * (bpp / 8));
			levels[i].size_per_layer = levels[i].pitch * cy;
			cx >>= 1;
			cy >>= 1;
			cx = glm::max(cx, 1);
			cy = glm::max(cy, 1);
			total_size += levels[i].size_per_layer * layer_count;
		}

		if (type == TextureTypeImage)
			layout = VK_IMAGE_LAYOUT_UNDEFINED;
		else
		{
			layout = format_type == FormatTypeColor ?
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			auto cb = begin_once_command_buffer();
			transition_layout(cb, VK_IMAGE_LAYOUT_UNDEFINED, layout);
			end_once_command_buffer(cb);
		}
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

	void Texture::set_data_from_format()
	{
		switch (format)
		{
			case VK_FORMAT_R8_UNORM:
				channel = 1;
				bpp = 8;
				break;
			case VK_FORMAT_R16_UNORM:
				channel = 1;
				bpp = 16;
				break;
			case VK_FORMAT_R8G8B8A8_UNORM:
				channel = 4;
				bpp = 32;
				break;
			case VK_FORMAT_R8G8B8A8_SRGB:
				channel = 4;
				bpp = 32;
				sRGB = true;
				break;
			case VK_FORMAT_B8G8R8A8_UNORM:
				channel = 4;
				bpp = 32;
				break;
			case VK_FORMAT_B8G8R8A8_SRGB:
				channel = 4;
				bpp = 32;
				break;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				channel = 4;
				bpp = 64;
				break;
			case VK_FORMAT_R16G16B16A16_UNORM:
				channel = 4;
				bpp = 64;
				break;
			case VK_FORMAT_D16_UNORM:
				channel = 1;
				bpp = 16;
				break;
			case VK_FORMAT_D32_SFLOAT:
				channel = 1;
				bpp = 32;
				break;
			case VK_FORMAT_D16_UNORM_S8_UINT:
				channel = 1;
				bpp = 24;
				break;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				channel = 1;
				bpp = 32;
				break;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				channel = 1;
				bpp = 40;
				break;
		}
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
