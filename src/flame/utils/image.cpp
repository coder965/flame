#define NOMINMAX
#include <FreeImage.h>
#include <flame/utils/filesystem.h>
#include <flame/utils/image.h>

namespace tke
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

	Image::Image(const std::string &filename)
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
		auto s = FreeImage_GetDIBSize(dib);
		s = calc_pitch(cx, bpp) * cy;
		data = std::make_unique<unsigned char[]>(s);
		memcpy(data.get(), FreeImage_GetBits(dib), s);
		FreeImage_Unload(dib);
	}

	Image::Image(int _cx, int _cy, int _channel, int _bpp, unsigned char *_data) :
		cx(_cx),
		cy(_cy),
		channel(_channel),
		bpp(_bpp)
	{
		const auto stride = bpp / 8;
		auto size = calc_pitch(cx * stride) * cy;
		data = std::make_unique<unsigned char[]>(size);
		if (_data)
			memcpy(data.get(), _data, size);
		else
			memset(data.get(), 0, size);
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

	void Image::save(const std::string &filename)
	{
		auto fif = FreeImage_GetFIFFromFilename(filename.c_str());
		auto dib = FreeImage_ConvertFromRawBits(data.get(), cx, cy, calc_pitch(cx * (bpp / 8)), bpp, 0x0000FF, 0xFF0000, 0x00FF00, true);
		FreeImage_Save(fif, dib, filename.c_str());
		FreeImage_Unload(dib);
	}

	Image * Image::create_distance_transform(int offset)
	{
		auto bound = std::numeric_limits<float>::min();	
		const auto stride = bpp / 8;
		auto pitch = calc_pitch(cx * stride);
		auto temp = std::make_unique<float[]>(cx * cy);
		for (auto y = 0; y < cy; y++)
		{
			auto line = data.get() + y * pitch;
			auto temp_line = temp.get() + y * cx;
			temp_line[0] = 0;
			auto i = 0;
			for (auto x = 1; x < cx - 1; x++)
			{
				i++;
				if (line[x * stride + offset] == 0)
				{
					temp[y * cx + x] = 0;
					i = 0;
				}
				else
					temp[y * cx + x] = i * i;
			}
			temp_line[cx - 1] = 0;
			i = 0;
			for (auto x = cx - 2; x > 0; x--)
			{
				i++;
				if (line[x * stride + offset] == 0)
				{
					temp[y * cx + x] = 0;
					i = 0;
				}
				else
					temp[y * cx + x] = i * i;
			}
		}
		for (auto x = 1; x < cx - 1; x++)
		{
			for (auto y = 0; y < cy; y++)
			{
				auto d = temp[y * cx + x];
				for (auto yy = 0; yy < cy; yy++)
				{
					if (yy == y)
						continue;
					auto dd = glm::pow(glm::abs(yy - y), 2) + temp[yy * cx + x];
					if (dd < d)
						d = dd;
				}
				temp[y * cx + x] = d;
				bound = glm::max(bound, d);
			}
		}
		auto i = new Image(cx, cy, 1, 8);
		auto alpha = new unsigned char(cx * cy);
		auto total_size = cx * cy;
		for (auto i = 0; i < total_size; i++)
			alpha[i] = (temp[i] / bound) * 255.f;
		return i;
	}
}
