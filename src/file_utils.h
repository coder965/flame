#pragma once

#include <fstream>
#include <filesystem>
#include <memory>

#include "math/math.h" // opt: should we not include this?

namespace std
{
	namespace fs = experimental::filesystem;
}

namespace tke
{
	void skip(std::ifstream &file, int byte_count);
	char read_char(std::ifstream &file);
	short read_short(std::ifstream &file);
	int read_int(std::ifstream &file);
	glm::ivec2 read_int2(std::ifstream &file);
	glm::ivec3 read_int3(std::ifstream &file);
	glm::ivec4 read_int4(std::ifstream &file);
	float read_float(std::ifstream &file);
	glm::vec2 read_float2(std::ifstream &file);
	glm::vec3 read_float3(std::ifstream &file);
	glm::vec4 read_float4(std::ifstream &file);
	std::string read_string(std::ifstream &file);

	void write_char(std::ofstream &file, char v);
	void write_short(std::ofstream &file, short v);
	void write_int(std::ofstream &file, int v);
	void write_int2(std::ofstream &file, const glm::ivec2 &v);
	void write_int3(std::ofstream &file, const glm::ivec3 &v);
	void write_int4(std::ofstream &file, const glm::ivec4 &v);
	void write_float(std::ofstream &file, float v);
	void write_float2(std::ofstream &file, const glm::vec2 &v);
	void write_float3(std::ofstream &file, const glm::vec3 &v);
	void write_float4(std::ofstream &file, const glm::vec4 &v);
	void write_string(std::ofstream &file, const std::string &v);

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

	std::string float_serialize(float v);

	struct XMLAttribute
	{
		std::string name;
		std::string value;

		XMLAttribute();
		XMLAttribute(const std::string &_name, int v);
		XMLAttribute(const std::string &_name, const glm::ivec2 &v);
		XMLAttribute(const std::string &_name, const glm::ivec3 &v);
		XMLAttribute(const std::string &_name, const glm::ivec4 &v);
		XMLAttribute(const std::string &_name, float v);
		XMLAttribute(const std::string &_name, const glm::vec2 &v);
		XMLAttribute(const std::string &_name, const glm::vec3 &v);
		XMLAttribute(const std::string &_name, const glm::vec4 &v);
		XMLAttribute(const std::string &_name, const std::string &v);

		void set(int v);
		void set(const glm::ivec2 &v);
		void set(const glm::ivec3 &v);
		void set(const glm::ivec4 &v);
		void set(float v);
		void set(const glm::vec2 &v);
		void set(const glm::vec3 &v);
		void set(const glm::vec4 &v);
		void set(const std::string &v);

		int get_int() const;
		glm::ivec2 get_int2() const;
		glm::ivec3 get_int3() const;
		glm::ivec4 get_int4() const;
		int get_float() const;
		glm::vec2 get_float2() const;
		glm::vec3 get_float3() const;
		glm::vec4 get_float4() const;
		std::string get_string() const;
	};

	struct XMLNode
	{
		std::string name;
		std::string content;
		std::vector<std::unique_ptr<XMLAttribute>> attributes;
		std::vector<std::unique_ptr<XMLNode>> children;

		void *ptr = nullptr;

		XMLNode(const std::string &_name);

		void add_attribute(XMLAttribute *a);
		XMLAttribute *first_attribute(const std::string &_name);

		void add_node(XMLNode *n);
		XMLNode *first_node(const std::string &_name);

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
