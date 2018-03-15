#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <flame/common/filesystem.h>
#include <flame/common/image.h>

namespace flame
{
	int calc_pitch(int cx)
	{
		auto pitch = cx;
		if (pitch % 4 == 0)
			return pitch;
		return pitch + 4 - pitch % 4;
	}

	int calc_pitch(int cx, int bpp)
	{
		return calc_pitch(cx * (bpp / 8));
	}

	Image::Image(const std::string &filename) :
		own_data(true)
	{
		auto ext = std::filesystem::path(filename).extension().string();
		stbi_set_flip_vertically_on_load(is_image_file(ext));
		auto img = stbi_load(filename.c_str(), &cx, &cy, &channel, 0);
		if (!img)
			assert(0); // format not support
		sRGB = false;
		bpp = channel * 8;
		pitch = calc_pitch(cx, bpp);
		auto size = pitch * cy;
		data = new unsigned char[size];
		memcpy(data, img, size);
		stbi_image_free(img);
	}

	Image::Image(int _cx, int _cy, int _channel, int _bpp, unsigned char *_data, bool _own_data) :
		cx(_cx),
		cy(_cy),
		channel(_channel),
		bpp(_bpp),
		pitch(calc_pitch(cx, bpp)),
		sRGB(false),
		own_data(_own_data)
	{
		const auto stride = bpp / 8;
		auto size = calc_pitch(cx * stride) * cy;
		if (own_data)
		{
			data = new unsigned char[size];
			if (_data)
				memcpy(data, _data, size);
			else
				memset(data, 0, size);
		}
		else
			data = _data;
	}

	Image::~Image()
	{
		if (own_data)
			delete[]data;
	}

	void Image::clear(float color)
	{
		assert(channel == 1);
	}

	void Image::clear(glm::vec2 color)
	{
		assert(channel == 2);
	}

	void Image::clear(glm::vec3 color)
	{
		assert(channel == 3);
	}

	void Image::clear(glm::vec4 color)
	{
		assert(channel == 4);
	}

	void Image::copy_to(Image *dst, int src_x, int src_y, int cx, int cy, int dst_x, int dst_y)
	{
		for (auto j = 0; j < cy; j++)
		{
			for (auto i = 0; i < cx; i++)
				dst->data[(dst_y + j) * dst->pitch + dst_x + i] = data[(src_y + j) * pitch + src_x + i];
		}
	}

	void Image::save(const std::string &filename)
	{
		stbi_write_png(filename.c_str(), cx, cy, channel, data, pitch);
	}

	void Image::save_as_raw_bit_rgba32(const std::string &filename)
	{
		assert(channel == 4);
		assert(bpp == 32);
		std::ofstream file(filename, std::ios::binary);
		write_int(file, cx);
		write_int(file, cy);
		file.write((char*)data, pitch * cy);
	}
}
