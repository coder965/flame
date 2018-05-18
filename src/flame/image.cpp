//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <flame/filesystem.h>
#include <flame/image.h>

namespace flame
{
	static void set_size(Image *i)
	{
		i->pitch = calc_pitch(i->cx, i->bpp);
		i->size = i->pitch * i->cy;
		i->data = new unsigned char[i->size];
	}

	void Image::add_alpha_channel()
	{
		assert(channel == 3);
		auto new_data = new unsigned char[cx * cy * 4];
		pitch = cx * 4;
		for (auto j = 0; j < cy; j++)
		{
			for (auto i = 0; i < cx; i++)
			{
				new_data[j * pitch + i * 4 + 0] = data[j * cx * 3 + i * 3 + 0];
				new_data[j * pitch + i * 4 + 1] = data[j * cx * 3 + i * 3 + 1];
				new_data[j * pitch + i * 4 + 2] = data[j * cx * 3 + i * 3 + 2];
				new_data[j * pitch + i * 4 + 3] = 255;
			}
		}
		channel = 4;
		bpp = 32;
		size = cx * 4 * cy;
		delete[]data;
		data = new_data;
	}

	void Image::swap_RB()
	{
		assert(channel > 2);
		for (auto j = 0; j < cy; j++)
		{
			for (auto i = 0; i < cx; i++)
				std::swap(data[j * pitch + i * channel + 0], data[j * pitch + i * channel + 2]);
		}
	}

	void Image::copy_to(Image *dst, int src_x, int src_y, int cx, int cy, int dst_x, int dst_y)
	{
		for (auto j = 0; j < cy; j++)
		{
			for (auto i = 0; i < cx; i++)
				dst->data[(dst_y + j) * dst->pitch + dst_x + i] = data[(src_y + j) * pitch + src_x + i];
		}
	}

	Image *create_image(int cx, int cy, int channel, int bpp)
	{
		auto i = new Image;
		i->cx = cx;
		i->cy = cy;
		i->channel = channel;
		i->bpp = bpp;
		i->sRGB = false;
		set_size(i);
		memset(i->data, 0, i->size);
		return i;
	}

	Image *load_image(const std::string &filename)
	{
		auto ext = std::filesystem::path(filename).extension().string();
		if (ext == ".rimg")
		{
			std::ifstream file(filename, std::ios::binary);
			if (!file.good())
				return nullptr;
			auto i = new Image;
			i->sRGB = false;
			i->cx = read<int>(file);
			i->cy = read<int>(file);
			i->channel = read<int>(file);
			i->bpp = i->channel * 8;
			set_size(i);
			file.read((char*)i->data, i->size);
			return i;
		}

		int cx, cy, channel;
		auto img = stbi_load(filename.c_str(), &cx, &cy, &channel, 0);
		if (!img)
			return nullptr;
		auto i = new Image;
		i->sRGB = false;
		i->cx = cx;
		i->cy = cy;
		i->channel = channel;
		i->bpp = channel * 8;
		set_size(i);
		memcpy(i->data, img, i->size);
		stbi_image_free(img);
		return i;
	}

	void save_image(int cx, int cy, int channel, int bpp, unsigned char *data, const std::string &filename)
	{
		auto pitch = calc_pitch(cx, bpp);
		auto ext = std::filesystem::path(filename).extension().string();
		if (ext == ".rimg")
		{
			std::ofstream file(filename, std::ios::binary);
			write(file, cx);
			write(file, cy);
			write(file, channel);
			file.write((char*)data, pitch * cy);
			return;
		}
		stbi_write_png(filename.c_str(), cx, cy, channel, data, pitch);
	}

	void release_image(Image *i)
	{
		delete[]i->data;
		delete i;
	}
}
