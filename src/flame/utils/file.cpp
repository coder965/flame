#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include <flame/utils/file.h>

namespace tke
{
	void skip(std::ifstream &file, int byte_count)
	{
		int length = file.tellg();
		file.seekg(length + byte_count);
	}

	char read_char(std::ifstream &file)
	{
		char v;
		file.read((char*)&v, sizeof(char));
		return v;
	}

	short read_short(std::ifstream &file)
	{
		short v;
		file.read((char*)&v, sizeof(short));
		return v;
	}

	int read_int(std::ifstream &file)
	{
		int v;
		file.read((char*)&v, sizeof(int));
		return v;
	}

	glm::ivec2 read_int2(std::ifstream &file)
	{
		glm::ivec2 v;
		file.read((char*)&v, sizeof(glm::ivec2));
		return v;
	}

	glm::ivec3 read_int3(std::ifstream &file)
	{
		glm::ivec3 v;
		file.read((char*)&v, sizeof(glm::ivec3));
		return v;
	}

	glm::ivec4 read_int4(std::ifstream &file)
	{
		glm::ivec4 v;
		file.read((char*)&v, sizeof(glm::ivec4));
		return v;
	}

	float read_float(std::ifstream &file)
	{
		float v;
		file.read((char*)&v, sizeof(float));
		return v;
	}

	glm::vec2 read_float2(std::ifstream &file)
	{
		glm::vec2 v;
		file.read((char*)&v, sizeof(glm::vec2));
		return v;
	}

	glm::vec3 read_float3(std::ifstream &file)
	{
		glm::vec3 v;
		file.read((char*)&v, sizeof(glm::vec3));
		return v;
	}

	glm::vec4 read_float4(std::ifstream &file)
	{
		glm::vec4 v;
		file.read((char*)&v, sizeof(glm::vec4));
		return v;
	}

	std::string read_string(std::ifstream &file)
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

	void write_char(std::ofstream &file, char v)
	{
		file.write((char*)&v, sizeof(char));
	}

	void write_short(std::ofstream &file, short v)
	{
		file.write((char*)&v, sizeof(short));
	}

	void write_int(std::ofstream &file, int v)
	{
		file.write((char*)&v, sizeof(int));
	}

	void write_int2(std::ofstream &file, const glm::ivec2 &v)
	{
		file.write((char*)&v, sizeof(glm::ivec2));
	}

	void write_int3(std::ofstream &file, const glm::ivec3 &v)
	{
		file.write((char*)&v, sizeof(glm::ivec3));
	}

	void write_int4(std::ofstream &file, const glm::ivec4 &v)
	{
		file.write((char*)&v, sizeof(glm::ivec4));
	}

	void write_float(std::ofstream &file, float v)
	{
		file.write((char*)&v, sizeof(float));
	}

	void write_float2(std::ofstream &file, const glm::vec2 &v)
	{
		file.write((char*)&v, sizeof(glm::vec2));
	}

	void write_float3(std::ofstream &file, const glm::vec3 &v)
	{
		file.write((char*)&v, sizeof(glm::vec3));
	}

	void write_float4(std::ofstream &file, const glm::vec4 &v)
	{
		file.write((char*)&v, sizeof(glm::vec4));
	}

	void write_string(std::ofstream &file, const std::string &v)
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

	bool is_text_file(const std::string &ext)
	{
		if (ext == ".txt" ||
			ext == ".h" || ext == ".c" || ext == ".cpp" || ext == ".hpp" || ext == ".cxx" || ext == ".inl" ||
			ext == ".glsl" || ext == ".vert" || ext == ".tesc" || ext == ".tese" || ext == ".geom" || ext == ".frag" || ext == ".hlsl" ||
			ext == ".xml" || ext == ".json" || ext == ".ini" || ext == ".log" ||
			ext == ".htm" || ext == ".html" || ext == ".css" ||
			ext == ".sln" || ext == ".vcxproj")
			return true;
		return false;
	}

	bool is_image_file(const std::string &ext)
	{
		if (ext == ".bmp" || ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" ||
			ext == ".tga" || ext == ".dds" || ext == ".ktx")
			return true;
		return false;
	}

	bool is_model_file(const std::string &ext)
	{
		if (ext == ".obj" || ext == ".pmd" || ext == ".pmx" || ext == ".tkm" || ext == ".dae")
			return true;
		return false;
	}

	bool is_terrain_file(const std::string &ext)
	{
		if (ext == ".tkt")
			return true;
		return false;
	}

	bool is_scene_file(const std::string &ext)
	{
		if (ext == ".tks")
			return true;
		return false;
	}

	FileType get_file_type(const std::string &ext)
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

	size_t get_file_length(std::ifstream &f)
	{
		f.seekg(0, std::ios::end);
		auto s = f.tellg();
		f.seekg(0, std::ios::beg);
		return s;
	}

	std::pair<std::unique_ptr<char[]>, size_t> get_file_content(const std::string &filename)
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

	std::string float_serialize(float v)
	{
		auto str = std::to_string(v);
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);
		return str;
	}

	XMLAttribute::XMLAttribute()
	{
	}

	XMLAttribute::XMLAttribute(const std::string &_name, bool v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, int v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const glm::ivec2 &v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const glm::ivec3 &v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const glm::ivec4 &v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, float v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const glm::vec2 &v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const glm::vec3 &v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const glm::vec4 &v) :
		name(_name)
	{
		set(v);
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const char *v) :
		name(_name), value(v)
	{
	}

	XMLAttribute::XMLAttribute(const std::string &_name, const std::string &v) :
		name(_name), value(v)
	{
	}

	void XMLAttribute::set(bool v)
	{
		value = v ? "true" : "false";
	}

	void XMLAttribute::set(int v)
	{
		value = std::to_string(v);
	}

	void XMLAttribute::set(const glm::ivec2 &v)
	{
		value = std::to_string(v.x) + "/" + std::to_string(v.y);
	}

	void XMLAttribute::set(const glm::ivec3 &v)
	{
		value = std::to_string(v.x) + "/" + std::to_string(v.y) + "/" + std::to_string(v.z);
	}

	void XMLAttribute::set(const glm::ivec4 &v)
	{
		value = std::to_string(v.x) + "/" + std::to_string(v.y) + "/" + std::to_string(v.z) + "/" + std::to_string(v.w);
	}

	void XMLAttribute::set(float v)
	{
		value = float_serialize(v);
	}

	void XMLAttribute::set(const glm::vec2 &v)
	{
		value = float_serialize(v.x) + "/" + float_serialize(v.y);
	}

	void XMLAttribute::set(const glm::vec3 &v)
	{
		value = float_serialize(v.x) + "/" + float_serialize(v.y) + "/" + float_serialize(v.z);
	}

	void XMLAttribute::set(const glm::vec4 &v)
	{
		value = float_serialize(v.x) + "/" + float_serialize(v.y) + "/" + float_serialize(v.z) + "/" + float_serialize(v.w);
	}

	void XMLAttribute::set(const std::string &v)
	{
		value = v;
	}

	bool XMLAttribute::get_bool() const
	{
		return value == "true";
	}

	int XMLAttribute::get_int() const
	{
		return std::stoi(value);
	}

	glm::ivec2 XMLAttribute::get_int2() const
	{
		glm::ivec2 v;
		assert(sscanf(value.c_str(), "%d/%d", &v.x, &v.y) == 2);
		return v;
	}

	glm::ivec3 XMLAttribute::get_int3() const
	{
		glm::ivec3 v;
		assert(sscanf(value.c_str(), "%d/%d/%d", &v.x, &v.y, &v.z) == 3);
		return v;
	}

	glm::ivec4 XMLAttribute::get_int4() const
	{
		glm::ivec4 v;
		assert(sscanf(value.c_str(), "%d/%d/%d/%d", &v.x, &v.y, &v.z, &v.w) == 4);
		return v;
	}

	float XMLAttribute::get_float() const
	{
		return std::stof(value);
	}

	glm::vec2 XMLAttribute::get_float2() const
	{
		glm::vec2 v;
		assert(sscanf(value.c_str(), "%f/%f", &v.x, &v.y) == 2);
		return v;
	}

	glm::vec3 XMLAttribute::get_float3() const
	{
		glm::vec3 v;
		assert(sscanf(value.c_str(), "%f/%f/%f", &v.x, &v.y, &v.z) == 3);
		return v;
	}

	glm::vec4 XMLAttribute::get_float4() const
	{
		glm::vec4 v;
		assert(sscanf(value.c_str(), "%f/%f/%f/%f", &v.x, &v.y, &v.z, &v.w) == 4);
		return v;
	}

	std::string XMLAttribute::get_string() const
	{
		return value;
	}

	XMLNode::XMLNode(const std::string &_name) : 
		name(_name)
	{
	}

	void XMLNode::add_attribute(XMLAttribute *a)
	{
		attributes.emplace_back(a);
	}

	XMLAttribute *XMLNode::first_attribute(const std::string &_name) const
	{
		for (auto &a : attributes)
		{
			if (a->name == _name)
				return a.get();
		}
		return nullptr;
	}

	void XMLNode::get_attribute_bool(const std::string &_name, bool &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_bool();
	}

	void XMLNode::get_attribute_int(const std::string &_name, int &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_int();
	}

	void XMLNode::get_attribute_int2(const std::string &_name, glm::ivec2 &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_int2();
	}

	void XMLNode::get_attribute_int3(const std::string &_name, glm::ivec3 &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_int3();
	}

	void XMLNode::get_attribute_int4(const std::string &_name, glm::ivec4 &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_int4();
	}

	void XMLNode::get_attribute_float(const std::string &_name, float &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_float();
	}

	void XMLNode::get_attribute_float2(const std::string &_name, glm::vec2 &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_float2();
	}

	void XMLNode::get_attribute_float3(const std::string &_name, glm::vec3 &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_float3();
	}

	void XMLNode::get_attribute_float4(const std::string &_name, glm::vec4 &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_float4();
	}

	void XMLNode::get_attribute_string(const std::string &_name, std::string &dst) const
	{
		auto a = first_attribute(_name);
		if (a)
			dst = a->get_string();
	}

	void XMLNode::add_node(XMLNode *n)
	{
		children.emplace_back(n);
	}

	XMLNode *XMLNode::first_node(const std::string &_name) const
	{
		for (auto &c : children)
		{
			if (c->name == _name)
				return c.get();
		}
		return nullptr;
	}

	// THESE TWO FUNCTIONS ARE SEALED SINCE 2018-01-11

	//void XMLNode::addAttributes(void *src, ReflectionBank *b)
	//{
	//	ptr = src;
	//	b->enumertateReflections([](Reflection *r, int offset, void *_data) {
	//		auto n = (XMLNode*)_data;

	//		auto a = n->newAttribute();
	//		a->name = r->name;

	//		if (r->what == Reflection::eVariable)
	//		{
	//			auto v = r->toVar();
	//			a->set(v->type, v->ptr((void*)((TK_LONG_PTR)n->ptr + offset)));
	//		}
	//		else if (r->what == Reflection::eEnum)
	//		{
	//			auto e = r->toEnu();
	//			auto v = *e->ptr(n->ptr);

	//			bool first = true;
	//			for (int i = 0; i < e->pEnum->items.size(); i++)
	//			{
	//				auto &item = e->pEnum->items[i];
	//				if (v & item.value)
	//				{
	//					if (!first)a->value += " ";
	//					a->value += item.name;
	//					first = false;
	//				}
	//			}
	//		}
	//	}, this, 0);
	//}

	//void XMLNode::obtainFromAttributes(void *dst, ReflectionBank *b)
	//{
	//	ptr = dst;
	//	for (auto &a : attributes)
	//	{
	//		auto r = b->findReflection(a->name, 0);
	//		if (!r.first)
	//		{
	//			printf("cannot find \"%s\" reflection from %s\n", a->name.c_str(), b->name.c_str());
	//			continue;
	//		}
	//		switch (r.first->what)
	//		{
	//		case Reflection::eVariable:
	//		{
	//			auto v = r.first->toVar();
	//			a->get(v->type, v->ptr((void*)((TK_LONG_PTR)dst + r.second)));
	//			break;
	//		}
	//		case Reflection::eEnum:
	//			r.first->toEnu()->pEnum->get(a->value, r.first->toEnu()->ptr(dst));
	//			break;
	//		}
	//	}
	//}

	XMLDoc::XMLDoc(const std::string &_name) : 
		XMLNode(_name)
	{
	}

	XMLDoc::XMLDoc(const std::string &_name, const std::string &_filename) : 
		XMLNode(_name)
	{
		load(_filename);
	}

	static void _loadXML(rapidxml::xml_node<> *src, XMLNode *dst)
	{
		dst->content = src->value();
		for (auto a = src->first_attribute(); a; a = a->next_attribute())
			dst->add_attribute(new XMLAttribute(a->name(), std::string(a->value())));

		for (auto n = src->first_node(); n; n = n->next_sibling())
		{
			auto c = new XMLNode(n->name());
			dst->add_node(c);
			_loadXML(n, c);
		}
	}

	void XMLDoc::load(const std::string &filename)
	{
		auto content = get_file_content(filename);
		if (!content.first)
		{
			good = false;
			return;
		}

		rapidxml::xml_document<> xmlDoc;
		xmlDoc.parse<0>(content.first.get());

		auto rootNode = xmlDoc.first_node(name.c_str());
		if (rootNode)
			_loadXML(rootNode, this);
	}

	static void _saveXML(rapidxml::xml_document<> &doc, rapidxml::xml_node<> *n, XMLNode *p)
	{
		for (auto &a : p->attributes)
			n->append_attribute(doc.allocate_attribute(a->name.c_str(), a->value.c_str()));

		for (auto &c : p->children)
		{
			auto node = doc.allocate_node(rapidxml::node_element, c->name.c_str());
			n->append_node(node);
			_saveXML(doc, node, c.get());
		}
	}

	void XMLDoc::save(const std::string &filename)
	{
		rapidxml::xml_document<> xmlDoc;
		auto rootNode = xmlDoc.allocate_node(rapidxml::node_element, name.c_str());
		xmlDoc.append_node(rootNode);
		_saveXML(xmlDoc, rootNode, this);

		std::string str;
		rapidxml::print(std::back_inserter(str), xmlDoc);

		std::ofstream file(filename);
		file.write(str.data(), str.size());
	}
}
