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
		unsigned int cx;
		unsigned int cy;
		unsigned int pitch;
		unsigned int size;
		std::unique_ptr<unsigned char[]> data;

		ImageDataLevel();
		void calc_pitch(int bpp);
		void calc_size();
		void set_data(unsigned char *_data);
		unsigned char get_r(float x, float y, int bpp);
		unsigned char get_a(float x, float y, int bpp);
	};

	struct ImageFile
	{
		ImageFileType type;
		unsigned int channel;
		unsigned int bpp;
		std::vector<std::unique_ptr<ImageDataLevel>> levels;
		unsigned int layer;
		unsigned int total_size;
		bool sRGB;

		ImageFile(int _level = 1);
		int get_cx(int _level = 0) const;
		int get_cy(int _level = 0) const;
		unsigned char *get_data(int _level = 0) const;
	};

	std::unique_ptr<ImageFile> create_image_file(const std::string &filename);
	void new_image_file(const std::string &filename, int cx, int cy, int bpp);
	void save_image_file(const std::string &filename, unsigned char *data, int cx, int cy, int bpp);
	void create_and_save_image_distance_transform(unsigned char *data, int cx, int cy, int offset, int stride, const std::string &filename);
}
