#pragma once

#include <vector>
#include <string>

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
		unsigned char *v = nullptr;
	};

	struct ImageData
	{
		ImageFileType file_type = ImageFileTypeNull;
		size_t byte_per_pixel = 0;
		size_t channel = 0;
		std::vector<ImageDataLevel> levels;
		int layer = 1;
		size_t total_size = 0;
		bool sRGB = false;

		ImageData();
		~ImageData();
	};

	ImageData *createImageData(const std::string &filename);
	void saveImageFile(const std::string &filename, unsigned char *data, int cx, int cy, int byte_per_pixel);
}
