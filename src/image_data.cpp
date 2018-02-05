#define NOMINMAX
#include "../../FreeImage/Dist/x64/FreeImage.h"
#include "../../gli/gli/gli.hpp"
#include "file_utils.h"

#include "image_data.h"

namespace tke
{
	ImageDataLevel::ImageDataLevel() :
		cx(0),
		cy(0),
		pitch(0),
		size(0)
	{
	}

	void ImageDataLevel::calc_pitch(int bpp)
	{
		pitch = PITCH(cx * (bpp / 8));
	}

	void ImageDataLevel::calc_size()
	{
		size = pitch * cy;
	}

	void ImageDataLevel::set_data(unsigned char *_data)
	{
		data = std::make_unique<unsigned char[]>(size);
		memcpy(data.get(), _data, size);
	}

	unsigned char ImageDataLevel::get_r(float _x, float _y, int bpp)
	{
		if (!data || _x < 0.f || _y < 0.f || _x >= cx || _y >= cy)
			return 0;

		auto x = glm::fract(_x);
		int X = glm::floor(_x);
		auto y = glm::fract(_y);
		int Y = glm::floor(_y);

#define gd(a, b) (float)data[(a) * (bpp / 8) + (b) * pitch]
		return glm::mix(glm::mix(gd(X, Y), gd(X + 1, Y), x), glm::mix(gd(X, Y + 1), gd(X + 1, Y + 1), x), y);
#undef gd
	}

	unsigned char ImageDataLevel::get_a(float _x, float _y, int bpp)
	{
		if (!data || _x < 0.f || _y < 0.f || _x >= cx || _y >= cy)
			return 0;

		auto x = glm::fract(_x);
		int X = glm::floor(_x);
		auto y = glm::fract(_y);
		int Y = glm::floor(_y);

#define gd(a, b) (float)data[(a) * (bpp / 8) + 3 + (b) * pitch]
		return glm::mix(glm::mix(gd(X, Y), gd(X + 1, Y), x), glm::mix(gd(X, Y + 1), gd(X + 1, Y + 1), x), y);
#undef gd
	}

	ImageData::ImageData(int _level) :
		file_type(ImageFileTypeNull),
		bpp(0),
		channel(0),
		layer(1),
		total_size(0),
		sRGB(false)
	{
		assert(_level >= 1);
		levels.resize(_level);
		for (int i = 0; i < _level; i++)
			levels[i] = std::make_unique<ImageDataLevel>();
	}

	int ImageData::get_cx(int _level) const
	{
		return levels[_level]->cx;
	}

	int ImageData::get_cy(int _level) const
	{
		return levels[_level]->cy;
	}

	unsigned char *ImageData::get_data(int _level) const
	{
		return levels[_level]->data.get();
	}

	std::unique_ptr<ImageData> create_image_data(const std::string &filename)
	{
		std::fs::path path(filename);
		if (!std::fs::exists(path))
			return nullptr;

		auto ext = path.extension().string();
		if (ext == ".ktx" || ext == ".dds")
		{
			gli::texture Texture = gli::load(filename);
			if (Texture.empty())
				return nullptr;

			gli::gl GL(gli::gl::PROFILE_GL33);
			gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
			auto Target = GL.translate(Texture.target());
			assert(!gli::is_compressed(Texture.format()) && Target == gli::TARGET_2D);
			assert(Format.External != gli::gl::EXTERNAL_NONE && Format.Type != gli::gl::TYPE_NONE);

			auto data = std::make_unique<ImageData>(Texture.levels());
			data->file_type = ext == ".ktx" ? ImageFileTypeKTX : ImageFileTypeDDS;
			switch (Format.External)
			{
				case gli::gl::EXTERNAL_RED:
					data->channel = 1;
					break;
				case gli::gl::EXTERNAL_RG:
					data->channel = 2;
					break;
				case gli::gl::EXTERNAL_RGB:
					data->channel = 3;
					break;
				case gli::gl::EXTERNAL_BGR:
					data->channel = 3;
					break;
				case gli::gl::EXTERNAL_RGBA:
					data->channel = 4;
					break;
				case gli::gl::EXTERNAL_BGRA:
					data->channel = 4;
					break;
				case gli::gl::EXTERNAL_RED_INTEGER:
					data->channel = 1;
					break;
				case gli::gl::EXTERNAL_RG_INTEGER:
					data->channel = 2;
					break;
				case gli::gl::EXTERNAL_RGB_INTEGER:
					data->channel = 3;
					break;
				case gli::gl::EXTERNAL_BGR_INTEGER:
					data->channel = 3;
					break;
				case gli::gl::EXTERNAL_RGBA_INTEGER:
					data->channel = 4;
					break;
				case gli::gl::EXTERNAL_BGRA_INTEGER:
					data->channel = 4;
					break;
				case gli::gl::EXTERNAL_DEPTH:
					// not supported yet
					return nullptr;
				case gli::gl::EXTERNAL_DEPTH_STENCIL:
					// not supported yet
					return nullptr;
				case gli::gl::EXTERNAL_STENCIL:
					// not supported yet
					return nullptr;
				case gli::gl::EXTERNAL_LUMINANCE:
					// not supported yet
					return nullptr;
				case gli::gl::EXTERNAL_ALPHA:
					data->channel = 1;
					break;
				case gli::gl::EXTERNAL_LUMINANCE_ALPHA:
					// not supported yet
					return nullptr;
				case gli::gl::EXTERNAL_SRGB_EXT:
					data->channel = 3;
					data->sRGB = true;
					break;
				case gli::gl::EXTERNAL_SRGB_ALPHA_EXT:
					data->channel = 4;
					data->sRGB = true;
					break;
			}
			switch (Format.Type)
			{
				case gli::gl::TYPE_I8:
					data->bpp = 8;
					break;
				case gli::gl::TYPE_U8:
					data->bpp = 8;
					break;
				case gli::gl::TYPE_I16:
					data->bpp = 16;
					break;
				case gli::gl::TYPE_U16:
					data->bpp = 16;
					break;
				case gli::gl::TYPE_I32:
					data->bpp = 32;
					break;
				case gli::gl::TYPE_U32:
					data->bpp = 32;
					break;
				case gli::gl::TYPE_I64:
					data->bpp = 64;
					break;
				case gli::gl::TYPE_F16:
					data->bpp = 16;
					break;
				case gli::gl::TYPE_F16_OES:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_F32:
					data->bpp = 32;
					break;
				case gli::gl::TYPE_F64:
					data->bpp = 64;
					break;
				case gli::gl::TYPE_UINT32_RGB9_E5_REV:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT32_RG11B10F_REV:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT8_RG3B2:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT8_RG3B2_REV:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT16_RGB5A1:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT16_RGB5A1_REV:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT16_R5G6B5:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT16_R5G6B5_REV:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT16_RGBA4:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT16_RGBA4_REV:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT32_RGBA8:
					data->bpp = 8;
					break;
				case gli::gl::TYPE_UINT32_RGBA8_REV:
					data->bpp = 8;
					break;
				case gli::gl::TYPE_UINT32_RGB10A2:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT32_RGB10A2_REV:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT8_RG4_REV_GTC:
					// not supported yet
					return nullptr;
				case gli::gl::TYPE_UINT16_A1RGB5_GTC:
					// not supported yet
					return nullptr;
			}
			for (int l = 0; l < Texture.levels(); l++)
			{
				glm::tvec3<size_t> Extent(Texture.extent(l));
				data->levels[l]->cx = Extent.x;
				data->levels[l]->cy = Extent.y;
				data->levels[l]->calc_pitch(data->bpp);
				data->levels[l]->calc_size();
				data->levels[l]->set_data((unsigned char *)Texture.data(0, 0, l));
			}

			return data;
		}

		auto fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(filename.c_str());
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(filename.c_str());
		if (fif == FIF_UNKNOWN)
		{
			// format not support
			return nullptr;
		}
		auto dib = FreeImage_Load(fif, filename.c_str());
		if (!dib)
			return nullptr;
		if (fif == FREE_IMAGE_FORMAT::FIF_JPEG || fif == FREE_IMAGE_FORMAT::FIF_TARGA || fif == FREE_IMAGE_FORMAT::FIF_PNG)
			FreeImage_FlipVertical(dib);

		auto data = std::make_unique<ImageData>();
		auto colorType = FreeImage_GetColorType(dib);
		switch (fif)
		{
			case FREE_IMAGE_FORMAT::FIF_BMP:
				data->file_type = ImageFileTypeBMP;
				break;
			case FREE_IMAGE_FORMAT::FIF_JPEG:
				data->file_type = ImageFileTypeJPEG;
				break;
			case FREE_IMAGE_FORMAT::FIF_PNG:
				data->file_type = ImageFileTypePNG;
				break;
			case FREE_IMAGE_FORMAT::FIF_TARGA:
				data->file_type = ImageFileTypeTARGA;
				break;
		}
		switch (colorType)
		{
			case FIC_MINISBLACK: case FIC_MINISWHITE:
				data->channel = 1;
				break;
			case FIC_RGB:
			{
				auto newDib = FreeImage_ConvertTo32Bits(dib);
				FreeImage_Unload(dib);
				dib = newDib;
				data->channel = 4;
				break;
			}
			case FIC_RGBALPHA:
				data->channel = 4;
				break;
		}
		data->bpp = FreeImage_GetBPP(dib);

		data->levels[0]->cx = FreeImage_GetWidth(dib);
		data->levels[0]->cy = FreeImage_GetHeight(dib);
		data->levels[0]->pitch = FreeImage_GetPitch(dib);
		data->levels[0]->calc_size();
		data->levels[0]->set_data(FreeImage_GetBits(dib));
		FreeImage_Unload(dib);

		return data;
	}

	void new_image_file(const std::string &filename, int cx, int cy, int bpp)
	{
		auto fif = FreeImage_GetFIFFromFilename(filename.c_str());
		auto dib = FreeImage_Allocate(cx, cy, bpp);
		FreeImage_Save(fif, dib, filename.c_str());
		FreeImage_Unload(dib);
	}

	void save_image_file(const std::string &filename, unsigned char *data, int cx, int cy, int bpp)
	{
		std::experimental::filesystem::path path(filename);
		auto ext = path.extension().string();
		if (ext == ".ktx")
			assert(0); // WIP

		auto fif = FreeImage_GetFIFFromFilename(filename.c_str());
		auto dib = FreeImage_ConvertFromRawBits(data, cx, cy, PITCH(cx * (bpp / 8)), bpp, 0x0000FF, 0xFF0000, 0x00FF00, true);
		FreeImage_Save(fif, dib, filename.c_str());
		FreeImage_Unload(dib);
	}
}
