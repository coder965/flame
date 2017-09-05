#define NOMINMAX
#include "../../FreeImage/Source/FreeImage.h"
#include "../../gli/gli/gli.hpp"
#include <filesystem>

#include "image_data.h"

namespace tke
{
	ImageData::ImageData()
	{
		levels.resize(1);
	}

	ImageData::~ImageData()
	{
		for (auto &l : levels)
			delete[]l.v;
	}

	ImageData *createImageData(const std::string &filename)
	{
		std::experimental::filesystem::path path(filename);
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

			auto data = new ImageData;
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
				assert(0); // not supported yet
				break;
			case gli::gl::EXTERNAL_DEPTH_STENCIL:
				assert(0); // not supported yet
				break;
			case gli::gl::EXTERNAL_STENCIL:
				assert(0); // not supported yet
				break;
			case gli::gl::EXTERNAL_LUMINANCE:
				assert(0); // not supported yet
				break;
			case gli::gl::EXTERNAL_ALPHA:
				data->channel = 1;
				break;
			case gli::gl::EXTERNAL_LUMINANCE_ALPHA:
				assert(0); // not supported yet
				break;
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
				data->byte_per_pixel = 1;
				break;
			case gli::gl::TYPE_U8:
				data->byte_per_pixel = 1;
				break;
			case gli::gl::TYPE_I16:
				data->byte_per_pixel = 2;
				break;
			case gli::gl::TYPE_U16:
				data->byte_per_pixel = 2;
				break;
			case gli::gl::TYPE_I32:
				data->byte_per_pixel = 4;
				break;
			case gli::gl::TYPE_U32:
				data->byte_per_pixel = 4;
				break;
			case gli::gl::TYPE_I64:
				data->byte_per_pixel = 8;
				break;
			case gli::gl::TYPE_F16:
				data->byte_per_pixel = 2;
				break;
			case gli::gl::TYPE_F16_OES:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_F32:
				data->byte_per_pixel = 4;
				break;
			case gli::gl::TYPE_F64:
				data->byte_per_pixel = 8;
				break;
			case gli::gl::TYPE_UINT32_RGB9_E5_REV:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT32_RG11B10F_REV:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT8_RG3B2:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT8_RG3B2_REV:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT16_RGB5A1:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT16_RGB5A1_REV:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT16_R5G6B5:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT16_R5G6B5_REV:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT16_RGBA4:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT16_RGBA4_REV:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT32_RGBA8:
				data->byte_per_pixel = 1;
				break;
			case gli::gl::TYPE_UINT32_RGBA8_REV:
				data->byte_per_pixel = 1;
				break;
			case gli::gl::TYPE_UINT32_RGB10A2:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT32_RGB10A2_REV:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT8_RG4_REV_GTC:
				assert(0); // not supported yet
				break;
			case gli::gl::TYPE_UINT16_A1RGB5_GTC:
				assert(0); // not supported yet
				break;
			}
			data->levels.resize(Texture.levels());

			for (int l = 0; l < Texture.levels(); l++)
			{
				glm::tvec3<size_t> Extent(Texture.extent(l));
				data->levels[l].cx = Extent.x;
				data->levels[l].cy = Extent.y;
				data->levels[l].pitch = PITCH(data->levels[l].cx * data->byte_per_pixel);
				data->levels[l].size = data->levels[l].pitch * data->levels[l].cy;
				data->levels[l].v = new unsigned char[data->levels[l].size];
				memcpy(data->levels[l].v, Texture.data(0, 0, l), data->levels[l].size);
			}

			return data;
		}

		auto fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(filename.c_str());
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(filename.c_str());
		if (fif == FIF_UNKNOWN)
		{
			// format not support or file not exist
			assert(false);
			return nullptr;
		}
		auto dib = FreeImage_Load(fif, filename.c_str());
		if (!dib)
			return nullptr;
		if (fif == FREE_IMAGE_FORMAT::FIF_JPEG || fif == FREE_IMAGE_FORMAT::FIF_TARGA || fif == FREE_IMAGE_FORMAT::FIF_PNG)
			FreeImage_FlipVertical(dib);

		auto data = new ImageData;
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
		}
		break;
		case FIC_RGBALPHA:
			data->channel = 4;
			break;
		}
		data->byte_per_pixel = FreeImage_GetBPP(dib) / 8;

		data->levels[0].cx = FreeImage_GetWidth(dib);
		data->levels[0].cy = FreeImage_GetHeight(dib);
		data->levels[0].pitch = FreeImage_GetPitch(dib);
		data->levels[0].size = data->levels[0].pitch * data->levels[0].cy;
		data->levels[0].v = new unsigned char[data->levels[0].size];
		memcpy(data->levels[0].v, FreeImage_GetBits(dib), data->levels[0].size);
		FreeImage_Unload(dib);

		if (data->channel == 4)
		{
			if (fif == FREE_IMAGE_FORMAT::FIF_BMP ||
				fif == FREE_IMAGE_FORMAT::FIF_TARGA ||
				fif == FREE_IMAGE_FORMAT::FIF_JPEG ||
				fif == FREE_IMAGE_FORMAT::FIF_PNG)
			{
				for (int y = 0; y < data->levels[0].cy; y++)
				{
					for (int x = 0; x < data->levels[0].cx; x++)
					{
						std::swap(data->levels[0].v[y * data->levels[0].pitch + x * 4 + 0],
							data->levels[0].v[y * data->levels[0].pitch + x * 4 + 2]);
					}
				}
			}
		}

		return data;
	}

	void saveImageFile(const std::string &filename, unsigned char *data, int cx, int cy, int byte_per_pixel)
	{
		{
			std::experimental::filesystem::path path(filename);
			auto ext = path.extension().string();
			if (ext == ".ktx")
			{

			}
		}

		auto fif = FreeImage_GetFIFFromFilename(filename.c_str());
		auto pitch = PITCH(cx * byte_per_pixel);
		if (byte_per_pixel == 4)
		{
			if (fif == FREE_IMAGE_FORMAT::FIF_BMP ||
				fif == FREE_IMAGE_FORMAT::FIF_TARGA ||
				fif == FREE_IMAGE_FORMAT::FIF_JPEG ||
				fif == FREE_IMAGE_FORMAT::FIF_PNG)
			{
				for (int y = 0; y < cy; y++)
				{
					for (int x = 0; x < cx; x++)
					{
						std::swap(data[y * pitch + x * 4 + 0],
							data[y * pitch + x * 4 + 2]);
					}
				}
			}
		}
		auto dib = FreeImage_ConvertFromRawBits(data, cx, cy, PITCH(cx * byte_per_pixel), byte_per_pixel * 8, 0x0000FF, 0xFF0000, 0x00FF00, true);
		FreeImage_Save(fif, dib, filename.c_str());
	}
}
