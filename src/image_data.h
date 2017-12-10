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
		size_t cx = 0;
		size_t cy = 0;
		size_t pitch = 0;
		size_t size = 0;
		std::unique_ptr<unsigned char[]> v;
	};

	struct ImageData
	{
		ImageFileType file_type = ImageFileTypeNull;
		size_t bpp = 0;
		size_t channel = 0;
		std::vector<ImageDataLevel> levels;
		int layer = 1;
		size_t total_size = 0;
		bool sRGB = false;

		ImageData();
	};

	std::shared_ptr<ImageData> createImageData(const std::string &filename);
	void newImageFile(const std::string &filename, int cx, int cy, int bpp);
	void saveImageFile(const std::string &filename, const ImageDataLevel &data, int bpp);
}
