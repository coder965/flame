#pragma once

#include <flame/common_exports.h>

#include <fstream>
#include <filesystem>
#include <memory>

#include <flame/common/math.h> // opt: should we not include this?

namespace std
{
	namespace filesystem = experimental::filesystem;
}

namespace flame
{
	FLAME_COMMON_EXPORTS void skip(std::ifstream &file, int byte_count);

	FLAME_COMMON_EXPORTS char read_char(std::ifstream &file);
	FLAME_COMMON_EXPORTS short read_short(std::ifstream &file);
	FLAME_COMMON_EXPORTS int read_int(std::ifstream &file);
	FLAME_COMMON_EXPORTS glm::ivec2 read_int2(std::ifstream &file);
	FLAME_COMMON_EXPORTS glm::ivec3 read_int3(std::ifstream &file);
	FLAME_COMMON_EXPORTS glm::ivec4 read_int4(std::ifstream &file);
	FLAME_COMMON_EXPORTS float read_float(std::ifstream &file);
	FLAME_COMMON_EXPORTS glm::vec2 read_float2(std::ifstream &file);
	FLAME_COMMON_EXPORTS glm::vec3 read_float3(std::ifstream &file);
	FLAME_COMMON_EXPORTS glm::vec4 read_float4(std::ifstream &file);
	FLAME_COMMON_EXPORTS std::string read_string(std::ifstream &file);

	FLAME_COMMON_EXPORTS void write_char(std::ofstream &file, char v);
	FLAME_COMMON_EXPORTS void write_short(std::ofstream &file, short v);
	FLAME_COMMON_EXPORTS void write_int(std::ofstream &file, int v);
	FLAME_COMMON_EXPORTS void write_int2(std::ofstream &file, const glm::ivec2 &v);
	FLAME_COMMON_EXPORTS void write_int3(std::ofstream &file, const glm::ivec3 &v);
	FLAME_COMMON_EXPORTS void write_int4(std::ofstream &file, const glm::ivec4 &v);
	FLAME_COMMON_EXPORTS void write_float(std::ofstream &file, float v);
	FLAME_COMMON_EXPORTS void write_float2(std::ofstream &file, const glm::vec2 &v);
	FLAME_COMMON_EXPORTS void write_float3(std::ofstream &file, const glm::vec3 &v);
	FLAME_COMMON_EXPORTS void write_float4(std::ofstream &file, const glm::vec4 &v);
	FLAME_COMMON_EXPORTS void write_string(std::ofstream &file, const std::string &v);

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

	FLAME_COMMON_EXPORTS bool is_text_file(const std::string &ext);
	FLAME_COMMON_EXPORTS bool is_image_file(const std::string &ext);
	FLAME_COMMON_EXPORTS bool is_model_file(const std::string &ext);
	FLAME_COMMON_EXPORTS bool is_terrain_file(const std::string &ext);
	FLAME_COMMON_EXPORTS bool is_scene_file(const std::string &ext);
	FLAME_COMMON_EXPORTS FileType get_file_type(const std::string &ext);

	FLAME_COMMON_EXPORTS long long get_file_length(std::ifstream &f);

	FLAME_COMMON_EXPORTS std::pair<std::unique_ptr<char[]>, size_t> get_file_content(const std::string &filename);

	FLAME_COMMON_EXPORTS std::string ftos(float v);

	FLAME_COMMON_EXPORTS struct XMLAttribute
	{
		std::string name;
		std::string value;

		XMLAttribute();
		XMLAttribute(const std::string &_name, bool v);
		XMLAttribute(const std::string &_name, int v);
		XMLAttribute(const std::string &_name, const glm::ivec2 &v);
		XMLAttribute(const std::string &_name, const glm::ivec3 &v);
		XMLAttribute(const std::string &_name, const glm::ivec4 &v);
		XMLAttribute(const std::string &_name, float v);
		XMLAttribute(const std::string &_name, const glm::vec2 &v);
		XMLAttribute(const std::string &_name, const glm::vec3 &v);
		XMLAttribute(const std::string &_name, const glm::vec4 &v);
		XMLAttribute(const std::string &_name, const char *v);
		XMLAttribute(const std::string &_name, const std::string &v);

		void set(bool v);
		void set(int v);
		void set(const glm::ivec2 &v);
		void set(const glm::ivec3 &v);
		void set(const glm::ivec4 &v);
		void set(float v);
		void set(const glm::vec2 &v);
		void set(const glm::vec3 &v);
		void set(const glm::vec4 &v);
		void set(const std::string &v);

		bool get_bool() const;
		int get_int() const;
		glm::ivec2 get_int2() const;
		glm::ivec3 get_int3() const;
		glm::ivec4 get_int4() const;
		float get_float() const;
		glm::vec2 get_float2() const;
		glm::vec3 get_float3() const;
		glm::vec4 get_float4() const;
		std::string get_string() const;
	};

	FLAME_COMMON_EXPORTS struct XMLNode
	{
		std::string name;
		std::string content;
		std::vector<std::unique_ptr<XMLAttribute>> attributes;
		std::vector<std::unique_ptr<XMLNode>> children;

		void *ptr = nullptr;

		XMLNode(const std::string &_name);

		void add_attribute(XMLAttribute *a);
		XMLAttribute *first_attribute(const std::string &_name) const;
		void get_attribute_bool(const std::string &_name, bool &dst) const;
		void get_attribute_int(const std::string &_name, int &dst) const;
		void get_attribute_int2(const std::string &_name, glm::ivec2 &dst) const;
		void get_attribute_int3(const std::string &_name, glm::ivec3 &dst) const;
		void get_attribute_int4(const std::string &_name, glm::ivec4 &dst) const;
		void get_attribute_float(const std::string &_name, float &dst) const;
		void get_attribute_float2(const std::string &_name, glm::vec2 &dst) const;
		void get_attribute_float3(const std::string &_name, glm::vec3 &dst) const;
		void get_attribute_float4(const std::string &_name, glm::vec4 &dst) const;
		void get_attribute_string(const std::string &_name, std::string &dst) const;

		void add_node(XMLNode *n);
		XMLNode *first_node(const std::string &_name) const;

		// THESE TWO FUNCTIONS ARE SEALED SINCE 2018-01-11
		//void addAttributes(void *src, ReflectionBank *b);
		//void obtainFromAttributes(void *dst, ReflectionBank *b);
	};

	FLAME_COMMON_EXPORTS struct XMLDoc : XMLNode
	{
		bool good = true;

		XMLDoc(const std::string &_name);
		XMLDoc(const std::string &_name, const std::string &_filename);
		void load(const std::string &filename);
		void save(const std::string &filename);
	};
}
