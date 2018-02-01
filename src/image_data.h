#pragma once

#include <vector>
#include <string>
#include <memory>

#define PITCH(x) (x % 4 == 0 ? x : 4 - x % 4 + x)

namespace tke
{
	enum ImageFileType
	{
		ImageFileTypeNull,
		ImageFileTypeBMP,
		ImageFileTypeJPEG,
		ImageFileTypePNG,
		ImageFileTypeTARGA,
		ImageFileTypeKTX,
		ImageFileTypeDDS
	};

	struct ImageDataLevel
	{
		size_t cx;
		size_t cy;
		size_t pitch;
		size_t size;
		std::unique_ptr<unsigned char[]> v;

		ImageDataLevel();
	};

	struct ImageData
	{
		ImageFileType file_type;
		size_t bpp;
		size_t channel;
		std::vector<ImageDataLevel> levels;
		int layer;
		size_t total_size;
		bool sRGB;

		ImageData();
	};

	std::unique_ptr<ImageData> createImageData(const std::string &filename);
	void newImageFile(const std::string &filename, int cx, int cy, int bpp);
	void saveImageFile(const std::string &filename, const ImageDataLevel &data, int bpp);
	void saveBitmap24(const std::string &filename, int cx, int cy, void *data);
	void saveBitmap32(const std::string &filename, int cx, int cy, void *data);
}
