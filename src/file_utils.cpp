#include <regex>

#include "../../../rapidxml-1.13\rapidxml.hpp"
#include "../../../rapidxml-1.13\rapidxml_utils.hpp"
#include "../../../rapidxml-1.13\rapidxml_print.hpp"

#include "math/math.h"
#include "file_utils.h"

std::ifstream& operator&(std::ifstream &file, int v)
{
	file.read((char*)&v, sizeof(int));
	return file;
}

std::ifstream& operator&(std::ifstream &file, glm::ivec2 &v)
{
	file.read((char*)&v, sizeof(glm::ivec2));
	return file;
}

std::ifstream& operator&(std::ifstream &file, glm::ivec3 &v)
{
	file.read((char*)&v, sizeof(glm::ivec3));
	return file;
}

std::ifstream& operator&(std::ifstream &file, glm::ivec4 &v)
{
	file.read((char*)&v, sizeof(glm::ivec4));
	return file;
}

std::ifstream& operator&(std::ifstream &file, float v)
{
	file.read((char*)&v, sizeof(float));
	return file;
}

std::ifstream& operator&(std::ifstream &file, glm::vec2 &v)
{
	file.read((char*)&v, sizeof(glm::vec2));
	return file;
}

std::ifstream& operator&(std::ifstream &file, glm::vec3 &v)
{
	file.read((char*)&v, sizeof(glm::vec3));
	return file;
}

std::ifstream& operator&(std::ifstream &file, glm::vec4 &v)
{
	file.read((char*)&v, sizeof(glm::vec4));
	return file;
}

std::ofstream& operator&(std::ofstream &file, int v)
{
	file.write((char*)&v, sizeof(int));
	return file;
}

std::ofstream& operator&(std::ofstream &file, const glm::ivec2 &v)
{
	file.write((char*)&v, sizeof(glm::ivec2));
	return file;
}

std::ofstream& operator&(std::ofstream &file, const glm::ivec3 &v)
{
	file.write((char*)&v, sizeof(glm::ivec3));
	return file;
}

std::ofstream& operator&(std::ofstream &file, const glm::ivec4 &v)
{
	file.write((char*)&v, sizeof(glm::ivec4));
	return file;
}

std::ofstream& operator&(std::ofstream &file, float v)
{
	file.write((char*)&v, sizeof(float));
	return file;
}

std::ofstream& operator&(std::ofstream &file, const glm::vec2 &v)
{
	file.write((char*)&v, sizeof(glm::vec2));
	return file;
}

std::ofstream& operator&(std::ofstream &file, const glm::vec3 &v)
{
	file.write((char*)&v, sizeof(glm::vec3));
	return file;
}

std::ofstream& operator&(std::ofstream &file, const glm::vec4 &v)
{
	file.write((char*)&v, sizeof(glm::vec4));
	return file;
}

namespace tke
{
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

	void XMLAttribute::set(const std::type_index &t, void *_v)
	{
		if (t == typeid(std::string))
			value = *(std::string*)_v;
		else if (t == typeid(int))
			value = std::to_string(*(int*)_v);
		else if (t == typeid(float))
		{
			value = std::to_string(*(float*)_v);
			value.erase(value.find_last_not_of('0') + 1, std::string::npos);
		}
		else if (t == typeid(bool))
			value = *(bool*)_v ? "true" : "false";

		else if (t == typeid(glm::vec3))
		{
			auto &v = *(glm::vec3*)_v;
			auto strX = std::to_string(v.x);
			strX.erase(strX.find_last_not_of('0') + 1, std::string::npos);
			auto strY = std::to_string(v.y);
			strY.erase(strY.find_last_not_of('0') + 1, std::string::npos);
			auto strZ = std::to_string(v.z);
			strZ.erase(strZ.find_last_not_of('0') + 1, std::string::npos);
			value = strX + "/" + strY + "/" + strZ;
		}
	}

	void XMLAttribute::get(const std::type_index &t, void *_v)
	{
		if (t == typeid(std::string))
			*(std::string*)_v = value;
		else if (t == typeid(int))
			*(int*)_v = std::stoi(value);
		else if (t == typeid(float))
			*(float*)_v = std::stof(value);
		else if (t == typeid(bool))
			*(bool*)_v = value == "true" ? true : false;
		else if (t == typeid(glm::vec3))
		{
			auto &v = *(glm::vec3*)_v;
			std::regex pattern(R"(([\+\-\.0-9]+)/([\+\-\.0-9]+)/([\+\-\.0-9]+))");
			std::smatch match;
			std::regex_search(value, match, pattern);
			v.x = std::stof(match[1].str());
			v.y = std::stof(match[2].str());
			v.z = std::stof(match[3].str());
		}
	}

	XMLNode::XMLNode(const std::string &_name)
		: name(_name)
	{}

	XMLAttribute *XMLNode::new_attribute()
	{
		auto a = new XMLAttribute;
		attributes.emplace_back(a);
		return a;
	}

	XMLAttribute *XMLNode::new_attribute(const std::string &n, const std::string &v)
	{
		auto a = new_attribute();
		a->name = n;
		a->value = v;
		return a;
	}

	XMLAttribute *XMLNode::new_attribute(const std::string &n, const char *v)
	{
		auto a = new_attribute();
		a->name = n;
		a->value = v;
		return a;
	}

	XMLAttribute *XMLNode::new_attribute(const std::string &n, char *v)
	{
		auto a = new_attribute();
		a->name = n;
		a->value = v;
		return a;
	}

	XMLNode *XMLNode::new_node(const std::string &_name)
	{
		auto n = new XMLNode(_name);
		children.emplace_back(n);
		return n;
	}

	XMLAttribute *XMLNode::first_attribute(const std::string &_name)
	{
		for (auto &a : attributes)
		{
			if (a->name == _name)
				return a.get();
		}
		return nullptr;
	}

	XMLNode *XMLNode::first_node(const std::string &_name)
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

	XMLDoc::XMLDoc(const std::string &_name)
		: XMLNode(_name)
	{}

	XMLDoc::XMLDoc(const std::string &_name, const std::string &_filename)
		: XMLNode(_name)
	{
		load(_filename);
	}

	static void _loadXML(rapidxml::xml_node<> *n, XMLNode *p)
	{
		p->value = n->value();
		for (auto a = n->first_attribute(); a; a = a->next_attribute())
		{
			auto _a = p->new_attribute();
			_a->name = a->name();
			_a->value = a->value();
		}

		for (auto nn = n->first_node(); nn; nn = nn->next_sibling())
			_loadXML(nn, p->new_node(nn->name()));
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
