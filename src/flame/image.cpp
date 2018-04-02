#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <flame/filesystem.h>
#include <flame/image.h>

namespace flame
{
	Image *load_image(const std::string &filename)
	{
		auto ext = std::filesystem::path(filename).extension().string();
		if (ext == ".rimg")
		{
			std::ifstream file(filename, std::ios::binary);
			if (!file.good())
				return nullptr;
			auto i = new Image;
			i->sRGB = false;
			i->cx = read<int>(file);
			i->cy = read<int>(file);
			i->channel = read<int>(file);
			i->bpp = i->channel * 8;
			i->calc_size();
			file.read((char*)i->data, i->size);
			return i;
		}

		int cx, cy, channel;
		auto img = stbi_load(filename.c_str(), &cx, &cy, &channel, 0);
		if (!img)
			return nullptr;
		auto i = new Image;
		i->sRGB = false;
		i->cx = cx;
		i->cy = cy;
		i->channel = channel;
		i->bpp = channel * 8;
		i->calc_size();
		memcpy(i->data, img, i->size);
		stbi_image_free(img);
		return i;
	}

	void save_image(int cx, int cy, int channel, int bpp, unsigned char *data, const std::string &filename)
	{
		auto pitch = calc_pitch(cx, bpp);
		auto ext = std::filesystem::path(filename).extension().string();
		if (ext == ".rimg")
		{
			std::ofstream file(filename, std::ios::binary);
			write(file, cx);
			write(file, cy);
			write(file, channel);
			file.write((char*)data, pitch * cy);
			return;
		}
		stbi_write_png(filename.c_str(), cx, cy, channel, data, pitch);
	}

	void release_image(Image *i)
	{
		delete[]i->data;
		delete i;
	}
}
