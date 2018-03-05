#define NOMINMAX
#include <FreeImage.h>
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
		auto fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(filename.c_str());
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(filename.c_str());
		if (fif == FIF_UNKNOWN)
			assert(0); // format not support
		auto dib = FreeImage_Load(fif, filename.c_str());
		if (!dib)
			assert(0);
		if (fif == FREE_IMAGE_FORMAT::FIF_JPEG || fif == FREE_IMAGE_FORMAT::FIF_TARGA || fif == FREE_IMAGE_FORMAT::FIF_PNG)
			FreeImage_FlipVertical(dib);
		cx = FreeImage_GetWidth(dib);
		cy = FreeImage_GetHeight(dib);
		sRGB = false;
		switch (FreeImage_GetColorType(dib))
		{
			case FIC_MINISBLACK: case FIC_MINISWHITE:
				channel = 1;
				break;
			case FIC_RGB:
			{
				// convert RGB to RGBA
				auto dib32 = FreeImage_ConvertTo32Bits(dib);
				FreeImage_Unload(dib);
				dib = dib32;
				channel = 4;
				break;
			}
			case FIC_RGBALPHA:
				channel = 4;
				break;
		}
		bpp = FreeImage_GetBPP(dib);
		auto size = FreeImage_GetDIBSize(dib);
		size = calc_pitch(cx, bpp) * cy;
		data = new unsigned char[size];
		memcpy(data, FreeImage_GetBits(dib), size);
		FreeImage_Unload(dib);
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
		auto fif = FreeImage_GetFIFFromFilename(filename.c_str());
		auto dib = FreeImage_ConvertFromRawBits(data, cx, cy, calc_pitch(cx * (bpp / 8)), bpp, 0x0000FF, 0xFF0000, 0x00FF00, true);
		FreeImage_Save(fif, dib, filename.c_str());
		FreeImage_Unload(dib);
	}
}
