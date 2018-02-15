#define NOMINMAX
#include "../../FreeImage/Dist/x64/FreeImage.h"
#include "../../gli/gli/gli.hpp"
#include "file_utils.h"

#include "image_utils.h"

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
		std::fs::path path(filename);

		auto ext = path.extension().string();
		if (ext == ".ktx" || ext == ".dds")
		{
			gli::texture Texture = gli::load(filename);
			if (Texture.empty())
				assert(0);

			gli::gl GL(gli::gl::PROFILE_GL33);
			auto const Format = GL.translate(Texture.format(), Texture.swizzles());
			auto Target = GL.translate(Texture.target());
			assert(!gli::is_compressed(Texture.format()) && Target == gli::TARGET_2D);
			assert(Format.External != gli::gl::EXTERNAL_NONE && Format.Type != gli::gl::TYPE_NONE);
			cx = Texture.extent().x;
			cy = Texture.extent().y;
			level = Texture.levels();
			layer = 1;
			sRGB = false;
			switch (Format.External)
			{
				case gli::gl::EXTERNAL_RED:
					channel = 1;
					break;
				case gli::gl::EXTERNAL_RG:
					channel = 2;
					break;
				case gli::gl::EXTERNAL_RGB:
					channel = 3;
					break;
				case gli::gl::EXTERNAL_BGR:
					channel = 3;
					break;
				case gli::gl::EXTERNAL_RGBA:
					channel = 4;
					break;
				case gli::gl::EXTERNAL_BGRA:
					channel = 4;
					break;
				case gli::gl::EXTERNAL_RED_INTEGER:
					channel = 1;
					break;
				case gli::gl::EXTERNAL_RG_INTEGER:
					channel = 2;
					break;
				case gli::gl::EXTERNAL_RGB_INTEGER:
					channel = 3;
					break;
				case gli::gl::EXTERNAL_BGR_INTEGER:
					channel = 3;
					break;
				case gli::gl::EXTERNAL_RGBA_INTEGER:
					channel = 4;
					break;
				case gli::gl::EXTERNAL_BGRA_INTEGER:
					channel = 4;
					break;
				case gli::gl::EXTERNAL_DEPTH:
					assert(0); // WIP
				case gli::gl::EXTERNAL_DEPTH_STENCIL:
					assert(0); // WIP
				case gli::gl::EXTERNAL_STENCIL:
					assert(0); // WIP
				case gli::gl::EXTERNAL_LUMINANCE:
					assert(0); // WIP
				case gli::gl::EXTERNAL_ALPHA:
					channel = 1;
					break;
				case gli::gl::EXTERNAL_LUMINANCE_ALPHA:
					assert(0); // WIP
				case gli::gl::EXTERNAL_SRGB_EXT:
					channel = 3;
					sRGB = true;
					break;
				case gli::gl::EXTERNAL_SRGB_ALPHA_EXT:
					channel = 4;
					sRGB = true;
					break;
			}
			switch (Format.Type)
			{
				case gli::gl::TYPE_I8:
					bpp = 8;
					break;
				case gli::gl::TYPE_U8:
					bpp = 8;
					break;
				case gli::gl::TYPE_I16:
					bpp = 16;
					break;
				case gli::gl::TYPE_U16:
					bpp = 16;
					break;
				case gli::gl::TYPE_I32:
					bpp = 32;
					break;
				case gli::gl::TYPE_U32:
					bpp = 32;
					break;
				case gli::gl::TYPE_I64:
					bpp = 64;
					break;
				case gli::gl::TYPE_F16:
					bpp = 16;
					break;
				case gli::gl::TYPE_F16_OES:
					assert(0); // WIP
				case gli::gl::TYPE_F32:
					bpp = 32;
					break;
				case gli::gl::TYPE_F64:
					bpp = 64;
					break;
				case gli::gl::TYPE_UINT32_RGB9_E5_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT32_RG11B10F_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT8_RG3B2:
					assert(0); // WIP
				case gli::gl::TYPE_UINT8_RG3B2_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGB5A1:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGB5A1_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_R5G6B5:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_R5G6B5_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGBA4:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_RGBA4_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT32_RGBA8:
					bpp = 8;
					break;
				case gli::gl::TYPE_UINT32_RGBA8_REV:
					bpp = 8;
					break;
				case gli::gl::TYPE_UINT32_RGB10A2:
					assert(0); // WIP
				case gli::gl::TYPE_UINT32_RGB10A2_REV:
					assert(0); // WIP
				case gli::gl::TYPE_UINT8_RG4_REV_GTC:
					assert(0); // WIP
				case gli::gl::TYPE_UINT16_A1RGB5_GTC:
					assert(0); // WIP
			}
			data = std::make_unique<unsigned char[]>(Texture.size());
			memcpy(data.get(), Texture.data(), Texture.size());
		}
		else
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
			level = 1;
			layer = 1;
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
	}

	Image::Image(int _cx, int _cy, int _channel, int _bpp, int _level, int _layer, unsigned char *_data = nullptr) :
		cx(_cx),
		cy(_cy),
		channel(_channel),
		bpp(_bpp),
		level(_level),
		layer(_layer)
	{
		assert(_level >= 1);
		assert(_layer >= 1);

		const auto stride = bpp / 8;
		auto size = 0;
		for (auto i = 0; i < level; i++)
		{
			size += calc_pitch(_cx * stride) * _cy;
			_cx = glm::max(1, _cx >> 1);
			_cy = glm::max(1, _cy >> 1);
		}
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

	int Image::get_cx(int _level) const
	{
		return glm::max(1, cx >> _level);
	}

	int Image::get_cy(int _level) const
	{
		return glm::max(1, cy >> _level);
	}

	unsigned char *Image::get_data(int _level) const
	{
		const auto stride = bpp / 8;
		auto offset = 0;
		auto _cx = cx;
		auto _cy = cy;
		for (auto i = 0; i < level; i++)
		{
			offset += calc_pitch(_cx * stride) * _cy;
			_cx = glm::max(1, _cx >> 1);
			_cy = glm::max(1, _cy >> 1);
		}
		return &data[offset];
	}

	void Image::save(const std::string &filename)
	{
		std::fs::path path(filename);
		auto ext = path.extension().string();
		if (ext == ".ktx" || ext == ".dds")
			assert(0); // WIP

		assert(level == 1);
		assert(layer == 1);

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
		auto i = new Image(cx, cy, 1, 8, 1, 1);
		auto alpha = new unsigned char(cx * cy);
		auto total_size = cx * cy;
		for (auto i = 0; i < total_size; i++)
			alpha[i] = (temp[i] / bound) * 255.f;
		return i;
	}
}
