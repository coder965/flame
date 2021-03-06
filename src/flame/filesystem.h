//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#ifdef _FLAME_FILESYSTEM_EXPORTS
#define FLAME_FILESYSTEM_EXPORTS __declspec(dllexport)
#else
#define FLAME_FILESYSTEM_EXPORTS __declspec(dllimport)
#endif

#include <fstream>
#include <filesystem>
#include <memory>
#include <stdarg.h>

namespace std
{
	namespace filesystem = experimental::filesystem;
}

namespace flame
{
	template<class T>
	inline T read(std::ifstream &file)
	{
		T v;
		file.read((char*)&v, sizeof(T));
		return v;
	}

	inline std::string read_string(std::ifstream &file)
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
		std::string v;
		v.resize(size);
		file.read((char*)v.data(), size);
		return v;
	}

	template<class T>
	inline void write(std::ofstream &file, const T &v)
	{
		file.write((char*)&v, sizeof(T));
	}

	inline void write_string(std::ofstream &file, const std::string &v)
	{
		int size = v.size();
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
		file.write((char*)v.data(), v.size());
	}

	inline void write_fmt(std::ofstream &file, const char *fmt, ...)
	{
		static char buffer[1024];

		va_list ap;
		va_start(ap, fmt);
		auto len = vsprintf(buffer, fmt, ap);
		va_end(ap);

		file.write(buffer, len);
	}

	enum FileType
	{
		FileTypeUnknown,
		FileTypeFolder,
		FileTypeText,
		FileTypeImage,
		FileTypeModel,
		FileTypeTerrain,
		FileTypeScene
	};

	inline bool is_text_file(const std::string &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == ".txt" ||
			ext == ".h" || ext == ".c" || ext == ".cpp" || ext == ".hpp" || ext == ".cxx" || ext == ".inl" ||
			ext == ".glsl" || ext == ".vert" || ext == ".tesc" || ext == ".tese" || ext == ".geom" || ext == ".frag" || ext == ".hlsl" ||
			ext == ".xml" || ext == ".json" || ext == ".ini" || ext == ".log" ||
			ext == ".htm" || ext == ".html" || ext == ".css" ||
			ext == ".sln" || ext == ".vcxproj")
			return true;
		return false;
	}

	inline bool is_image_file(const std::string &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == ".bmp" || ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" ||
			ext == ".tga" || ext == ".dds" || ext == ".ktx")
			return true;
		return false;
	}

	inline bool is_model_file(const std::string &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == ".obj" || ext == ".pmd" || ext == ".pmx" || ext == ".tkm" || ext == ".dae")
			return true;
		return false;
	}

	inline bool is_terrain_file(const std::string &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == ".tkt")
			return true;
		return false;
	}

	inline bool is_scene_file(const std::string &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == ".tks")
			return true;
		return false;
	}

	inline FileType get_file_type(const std::string &ext)
	{
		if (is_text_file(ext))
			return FileTypeText;
		if (is_image_file(ext))
			return FileTypeImage;
		if (is_model_file(ext))
			return FileTypeModel;
		if (is_terrain_file(ext))
			return FileTypeTerrain;
		if (is_scene_file(ext))
			return FileTypeScene;
		return FileTypeUnknown;
	}

	inline long long get_file_length(std::ifstream &f)
	{
		f.seekg(0, std::ios::end);
		auto s = f.tellg();
		f.seekg(0, std::ios::beg);
		return s;
	}

	inline std::pair<std::unique_ptr<char[]>, size_t> get_file_content(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
			return std::make_pair(nullptr, 0);

		auto length = get_file_length(file);
		auto data = new char[length + 1];
		file.read(data, length);
		data[length] = 0;
		return std::make_pair(std::unique_ptr<char[]>(data), length);
	}

	struct XMLAttribute
	{
		std::string name;
		std::string value;

		XMLAttribute() {}

		XMLAttribute(const std::string &_name = "", const std::string &_value = "") :
			name(_name),
			value(_value)
		{
		}
	};

	struct XMLNode
	{
		std::string name;
		std::string content;
		std::vector<std::unique_ptr<XMLAttribute>> attributes;
		std::vector<std::unique_ptr<XMLNode>> children;

		void *ptr;

		XMLNode(const std::string &_name) :
			name(_name),
			ptr(nullptr)
		{
		}

		XMLAttribute *find_attribute(const std::string &_name) const
		{
			for (auto &a : attributes)
			{
				if (a->name == _name)
					return a.get();
			}
			return nullptr;
		}

		XMLNode *find_node(const std::string &_name) const
		{
			for (auto &c : children)
			{
				if (c->name == _name)
					return c.get();
			}
			return nullptr;
		}

		// THESE TWO FUNCTIONS ARE SEALED SINCE 2018-01-11
		//void addAttributes(void *src, ReflectionBank *b);
		//void obtainFromAttributes(void *dst, ReflectionBank *b);
	};

	struct XMLDoc : XMLNode
	{
		XMLDoc(const std::string &_name) :
			XMLNode(_name)
		{
		}
	};

	FLAME_FILESYSTEM_EXPORTS XMLDoc *load_xml(const std::string &_name, const std::string &filename);
	FLAME_FILESYSTEM_EXPORTS void save_xml(XMLDoc *doc, const std::string &filename);
	FLAME_FILESYSTEM_EXPORTS void release_xml(XMLDoc *doc);
}
