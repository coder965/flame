#pragma once

#include <flame/exports.h>

#include <fstream>
#include <filesystem>
#include <memory>

#include <flame/math.h> // opt: should we not include this?

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

	inline std::string ftos(float v)
	{
		auto str = std::to_string(v);
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);
		return str;
	}

	struct XMLAttribute
	{
		std::string name;
		std::string value;

		XMLAttribute() {}

		XMLAttribute(const std::string &_name, bool v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, int v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, const glm::ivec2 &v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, const glm::ivec3 &v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, const glm::ivec4 &v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, float v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, const glm::vec2 &v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, const glm::vec3 &v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, const glm::vec4 &v) :
			name(_name)
		{
			set(v);
		}

		XMLAttribute(const std::string &_name, const char *v) :
			name(_name), value(v)
		{
		}

		XMLAttribute(const std::string &_name, const std::string &v) :
			name(_name), value(v)
		{
		}

		void set(bool v)
		{
			value = v ? "true" : "false";
		}

		void set(int v)
		{
			value = std::to_string(v);
		}

		void set(const glm::ivec2 &v)
		{
			value = std::to_string(v.x) + "/" + std::to_string(v.y);
		}

		void set(const glm::ivec3 &v)
		{
			value = std::to_string(v.x) + "/" + std::to_string(v.y) + "/" + std::to_string(v.z);
		}

		void set(const glm::ivec4 &v)
		{
			value = std::to_string(v.x) + "/" + std::to_string(v.y) + "/" + std::to_string(v.z) + "/" + std::to_string(v.w);
		}

		void set(float v)
		{
			value = ftos(v);
		}

		void set(const glm::vec2 &v)
		{
			value = ftos(v.x) + "/" + ftos(v.y);
		}

		void set(const glm::vec3 &v)
		{
			value = ftos(v.x) + "/" + ftos(v.y) + "/" + ftos(v.z);
		}

		void set(const glm::vec4 &v)
		{
			value = ftos(v.x) + "/" + ftos(v.y) + "/" + ftos(v.z) + "/" + ftos(v.w);
		}

		void set(const std::string &v)
		{
			value = v;
		}

		bool get_bool() const
		{
			return value == "true";
		}

		int get_int() const
		{
			return std::stoi(value);
		}

		glm::ivec2 get_int2() const
		{
			glm::ivec2 v;
			assert(sscanf(value.c_str(), "%d/%d", &v.x, &v.y) == 2);
			return v;
		}

		glm::ivec3 get_int3() const
		{
			glm::ivec3 v;
			assert(sscanf(value.c_str(), "%d/%d/%d", &v.x, &v.y, &v.z) == 3);
			return v;
		}

		glm::ivec4 get_int4() const
		{
			glm::ivec4 v;
			assert(sscanf(value.c_str(), "%d/%d/%d/%d", &v.x, &v.y, &v.z, &v.w) == 4);
			return v;
		}

		float get_float() const
		{
			return std::stof(value);
		}

		glm::vec2 get_float2() const
		{
			glm::vec2 v;
			assert(sscanf(value.c_str(), "%f/%f", &v.x, &v.y) == 2);
			return v;
		}

		glm::vec3 get_float3() const
		{
			glm::vec3 v;
			assert(sscanf(value.c_str(), "%f/%f/%f", &v.x, &v.y, &v.z) == 3);
			return v;
		}

		glm::vec4 get_float4() const
		{
			glm::vec4 v;
			assert(sscanf(value.c_str(), "%f/%f/%f/%f", &v.x, &v.y, &v.z, &v.w) == 4);
			return v;
		}

		std::string get_string() const
		{
			return value;
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

		void add_attribute(XMLAttribute *a)
		{
			attributes.emplace_back(a);
		}

		XMLAttribute *first_attribute(const std::string &_name) const
		{
			for (auto &a : attributes)
			{
				if (a->name == _name)
					return a.get();
			}
			return nullptr;
		}

		void get_attribute_bool(const std::string &_name, bool &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_bool();
		}

		void get_attribute_int(const std::string &_name, int &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_int();
		}

		void get_attribute_int2(const std::string &_name, glm::ivec2 &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_int2();
		}

		void get_attribute_int3(const std::string &_name, glm::ivec3 &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_int3();
		}

		void get_attribute_int4(const std::string &_name, glm::ivec4 &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_int4();
		}

		void get_attribute_float(const std::string &_name, float &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_float();
		}

		void get_attribute_float2(const std::string &_name, glm::vec2 &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_float2();
		}

		void get_attribute_float3(const std::string &_name, glm::vec3 &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_float3();
		}

		void get_attribute_float4(const std::string &_name, glm::vec4 &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_float4();
		}

		void get_attribute_string(const std::string &_name, std::string &dst) const
		{
			auto a = first_attribute(_name);
			if (a)
				dst = a->get_string();
		}

		void add_node(XMLNode *n)
		{
			children.emplace_back(n);
		}

		XMLNode *first_node(const std::string &_name) const
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

	FLAME_EXPORTS XMLDoc *load_xml(const std::string &_name, const std::string &filename);
	FLAME_EXPORTS void save_xml(XMLDoc *doc, const std::string &filename);
	FLAME_EXPORTS void release_xml(XMLDoc *doc);
}
