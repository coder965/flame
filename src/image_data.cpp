#define NOMINMAX
#include "../../FreeImage/Dist/x64/FreeImage.h"
#include "../../gli/gli/gli.hpp"
#include <filesystem>

#include "image_data.h"

namespace tke
{
	ImageData::ImageData()
	{
		levels.resize(1);
	}

	std::unique_ptr<ImageData> createImageData(const std::string &filename)
	{
		std::experimental::filesystem::path path(filename);
		if (!std::experimental::filesystem::exists(path))
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

			auto data = std::make_unique<ImageData>();
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
			data->levels.resize(Texture.levels());

			auto pixel_size = data->bpp / 8;
			for (int l = 0; l < Texture.levels(); l++)
			{
				glm::tvec3<size_t> Extent(Texture.extent(l));
				data->levels[l].cx = Extent.x;
				data->levels[l].cy = Extent.y;
				data->levels[l].pitch = PITCH(data->levels[l].cx * pixel_size);
				data->levels[l].size = data->levels[l].pitch * data->levels[l].cy;
				data->levels[l].v = std::make_unique<unsigned char[]>(data->levels[l].size);
				memcpy(data->levels[l].v.get(), Texture.data(0, 0, l), data->levels[l].size);
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

		data->levels[0].cx = FreeImage_GetWidth(dib);
		data->levels[0].cy = FreeImage_GetHeight(dib);
		data->levels[0].pitch = FreeImage_GetPitch(dib);
		data->levels[0].size = data->levels[0].pitch * data->levels[0].cy;
		data->levels[0].v = std::make_unique<unsigned char[]>(data->levels[0].size);
		memcpy(data->levels[0].v.get(), FreeImage_GetBits(dib), data->levels[0].size);
		FreeImage_Unload(dib);

		return data;
	}

	void newImageFile(const std::string &filename, int cx, int cy, int bpp)
	{
		auto fif = FreeImage_GetFIFFromFilename(filename.c_str());
		auto dib = FreeImage_Allocate(cx, cy, bpp);
		FreeImage_Save(fif, dib, filename.c_str());
		FreeImage_Unload(dib);
	}

	void saveImageFile(const std::string &filename, const ImageDataLevel &data, int bpp)
	{
		std::experimental::filesystem::path path(filename);
		auto ext = path.extension().string();
		if (ext == ".ktx")
		{

		}

		auto fif = FreeImage_GetFIFFromFilename(filename.c_str());
		auto dib = FreeImage_ConvertFromRawBits(data.v.get(), data.cx, data.cy, data.pitch, bpp, 0x0000FF, 0xFF0000, 0x00FF00, true);
		FreeImage_Save(fif, dib, filename.c_str());
		FreeImage_Unload(dib);
	}

	void saveBitmap24(const std::string &filename, int width, int height, void *data)
	{
		auto dib = FreeImage_Allocate(width, height, 24);
		FreeImage_Save(FIF_BMP, dib, filename.c_str());
		FreeImage_Unload(dib);
	}

	void saveBitmap32(const std::string &filename, int width, int height, void *data)
	{
		auto dib = FreeImage_Allocate(width, height, 32);
		FreeImage_Save(FIF_BMP, dib, filename.c_str());
		FreeImage_Unload(dib);
	}
}
