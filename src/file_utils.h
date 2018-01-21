#pragma once

#include <fstream>
#include <filesystem>
#include <memory>
#include <typeindex>

namespace glm
{
	struct ivec2;
	struct ivec3;
	struct ivec4;
	struct vec2;
	struct vec3;
	struct vec4;
}

namespace tke
{
	int read_int(std::ifstream &file);
	glm::ivec2 read_int2(std::ifstream &file);
	glm::ivec3 read_int3(std::ifstream &file);
	glm::ivec4 read_int4(std::ifstream &file);
	float read_float(std::ifstream &file);
	glm::vec2 read_float2(std::ifstream &file);
	glm::vec3 read_float3(std::ifstream &file);
	glm::vec4 read_float4(std::ifstream &file);
	std::string read_string(std::ifstream &file);

	void write_int(std::ifstream &file, int v);
	void write_int2(std::ifstream &file, const glm::ivec2 &v);
	void write_int3(std::ifstream &file, const glm::ivec3 &v);
	void write_int4(std::ifstream &file, const glm::ivec4 &v);
	void write_float(std::ifstream &file, float v);
	void write_float2(std::ifstream &file, const glm::vec2 &v);
	void write_float3(std::ifstream &file, const glm::vec3 &v);
	void write_float4(std::ifstream &file, const glm::vec4 &v);
	void write_string(std::ifstream &file, const std::string &v);
}

inline std::ifstream& operator&(std::ifstream &file, std::string &str)
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
	namespace fs = experimental::filesystem;
}

namespace tke
{
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

	bool is_text_file(const std::string &ext);
	bool is_image_file(const std::string &ext);
	bool is_model_file(const std::string &ext);
	bool is_terrain_file(const std::string &ext);
	bool is_scene_file(const std::string &ext);
	FileType get_file_type(const std::string &ext);

	size_t get_file_length(std::ifstream &f);

	std::pair<std::unique_ptr<char[]>, size_t> get_file_content(const std::string &filename);

	struct XMLAttribute
	{
		std::string name;
		std::string value;

		void set(const std::type_index &t, void *v);

		template<class T>
		inline void set(T *v)
		{
			set(typeid(T), v);
		}

		template<class T>
		inline void set(const std::string &n, T *v)
		{
			name = n;

			set(v);
		}

		void get(const std::type_index &t, void *v);

		template<class T>
		inline void get(T *v)
		{
			get(typeid(T), v);
		}
	};

	struct XMLNode
	{
		void *ptr = nullptr;
		std::string name;
		std::string value;
		std::vector<std::unique_ptr<XMLAttribute>> attributes;
		std::vector<std::unique_ptr<XMLNode>> children;

		XMLNode(const std::string &_name);
		XMLAttribute *new_attribute();
		XMLAttribute *new_attribute(const std::string &, const std::string &);
		XMLAttribute *new_attribute(const std::string &, const char *);
		XMLAttribute *new_attribute(const std::string &, char *);

		template<class T>
		XMLAttribute *new_attribute(const std::string &n, T *v)
		{
			auto a = new_attribute();
			a->set(v);
			return a;
		}

		XMLNode *new_node(const std::string &_name);
		XMLAttribute *first_attribute(const std::string &_name);
		XMLNode *first_node(const std::string &_name);

		template <class... _Valty>
		inline void add_attribute(_Valty&&... _Val)
		{
			new_attribute(_Val...);
		}

		// THESE TWO FUNCTIONS ARE SEALED SINCE 2018-01-11
		//void addAttributes(void *src, ReflectionBank *b);
		//void obtainFromAttributes(void *dst, ReflectionBank *b);
	};

	struct XMLDoc : XMLNode
	{
		bool good = true;

		XMLDoc(const std::string &_name);
		XMLDoc(const std::string &_name, const std::string &_filename);
		void load(const std::string &filename);
		void save(const std::string &filename);
	};
}
