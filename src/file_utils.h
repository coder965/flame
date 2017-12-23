#pragma once

#include <fstream>
#include <filesystem>
#include <memory>

template<class T>
inline std::ifstream& operator&(std::ifstream &file, T &v)
{
	file.read((char*)&v, sizeof(T));
	return file;
}

template<class T>
inline std::ofstream& operator&(std::ofstream &file, T &v)
{
	file.write((char*)&v, sizeof(T));
	return file;
}

inline std::ifstream& operator>(std::ifstream &file, std::string &str)
{
	int size = 0;
	int q = 1;
	for (int i = 0; i < 4; i++)
	{
		unsigned char byte;
		file.read((char*)&byte, 1);
		if (byte >= 128)
			byte -= 128;
		else
			i = 4;
		size += q * byte;
		q *= 128;
	}
	str.resize(size);
	file.read((char*)str.data(), size);
	return file;
}

inline std::ofstream& operator<(std::ofstream &file, std::string &str)
{
	int size = str.size();
	for (int i = 0; i < 4; i++)
	{
		unsigned char byte = size % 128;
		size /= 128;
		if (size > 0)
			byte += 128;
		else
			i = 4;
		file.write((char*)&byte, 1);

	}
	file.write((char*)str.data(), str.size());
	return file;
}

namespace std
{
	namespace fs = std::experimental::filesystem;
}

namespace tke
{
	enum FileType
	{
		FileTypeUnknown,
		FileTypeText,
		FileTypeImage,
		FileTypeModel,
		FileTypeTerrain,
		FileTypeScene
	};

	bool is_text_file(const std::string &ext);
	bool is_image_file(const std::string &ext);
	bool is_model_file(const std::string &ext);
	bool is_terrain_file(const std::string &ext);
	bool is_scene_file(const std::string &ext);

	size_t get_file_length(std::ifstream &f);

	std::pair<std::unique_ptr<char[]>, size_t> get_file_content(const std::string &filename);
}
