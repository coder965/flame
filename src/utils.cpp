#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <regex>
#define NOMINMAX
#include <Windows.h>

#include "..\..\..\rapidxml-1.13\rapidxml.hpp"
#include "..\..\..\rapidxml-1.13\rapidxml_utils.hpp"
#include "..\..\..\rapidxml-1.13\rapidxml_print.hpp"

#include "math.h"
#include "utils.h"

// hash

unsigned int _HASH(char const * str, unsigned int seed)
{
	return 0 == *str ? seed : _HASH(str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

namespace tke
{
	int lineNumber(const char *str)
	{
		int lineNumber = 0;
		while (*str)
		{
			if (*str == '\n')
				lineNumber++;
			str++;
		}
		return lineNumber;
	}

	const char *getErrorString(Err errNum)
	{
		switch (errNum)
		{
		case Err::eNoErr:
			return "No error.";
		case Err::eInvalidEnum:
			return "Invalid enum.";
		case Err::eInvalidValue:
			return "Invalid value.";
		case Err::eInvalidOperation:
			return "Invalid operation.";
		case Err::eOutOfMemory:
			return "Out of memory.";
		case Err::eContextLost:
			return "Context lost.";
		case Err::eResourceLost:
			return "Resource lost.";
		default:
			return "unknow error";
		}
	}

	void *hInst;
	int screenCx;
	int screenCy;
	std::string exePath;

	const char *getClipBoard()
	{
		static std::string str;
		OpenClipboard(NULL);
		auto hClipMemory = ::GetClipboardData(CF_TEXT);
		auto dwLength = GlobalSize(hClipMemory);
		auto lpClipMemory = (LPBYTE)GlobalLock(hClipMemory);
		str =  (char*)lpClipMemory;
		GlobalUnlock(hClipMemory);
		CloseClipboard();
		return str.c_str();
	}

	void setClipBoard(const std::string &s)
	{
		auto hGlobalMemory = GlobalAlloc(GHND, s.size() + 1);
		strcpy((char*)GlobalLock(hGlobalMemory), s.c_str());
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hGlobalMemory);
		CloseClipboard();
	}

	std::string translate(int srcCP, int dstCP, const std::string &src)
	{
		auto wbuf = new wchar_t[src.size() + 1];
		MultiByteToWideChar(srcCP, 0, src.c_str(), -1, wbuf, src.size() + 1);
		auto buf = new char[src.size() + 1];
		WideCharToMultiByte(dstCP, 0, wbuf, -1, buf, src.size() + 1, NULL, false);
		delete[]wbuf;
		std::string str(buf);
		delete[]buf;
		return str;
	}

	std::string japaneseToChinese(const std::string &src)
	{
		return translate(932, 936, src);
	}

	void saveBitmap24(const std::string &filename, int width, int height, void *data)
	{
		auto imageSize = PITCH(width * 3) * height;
		BITMAPFILEHEADER header = {};
		header.bfType = 0x4D42;
		header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize;
		header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		BITMAPINFOHEADER info = {};
		info.biSize = sizeof(BITMAPINFOHEADER);
		info.biWidth = width;
		info.biHeight = height;
		info.biPlanes = 1;
		info.biBitCount = 24;
		info.biCompression = BI_RGB;
		std::ofstream file(filename);
		file.write((char*)&header, sizeof(BITMAPFILEHEADER));
		file.write((char*)&info, sizeof(BITMAPINFOHEADER));
		file.write((char*)data, imageSize);
	}

	void saveBitmap32(const std::string &filename, int width, int height, void *data)
	{
		auto imageSize = PITCH(width * 4) * height;
		BITMAPFILEHEADER header = {};
		header.bfType = 0x4D42;
		header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize;
		header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		BITMAPINFOHEADER info = {};
		info.biSize = sizeof(BITMAPINFOHEADER);
		info.biWidth = width;
		info.biHeight = height;
		info.biPlanes = 1;
		info.biBitCount = 32;
		info.biCompression = BI_RGB;
		std::ofstream file(filename);
		file.write((char*)&header, sizeof(BITMAPFILEHEADER));
		file.write((char*)&info, sizeof(BITMAPINFOHEADER));
		file.write((char*)data, imageSize);
	}

	void exec(const std::string &filename, const std::string &parameters)
	{
		SHELLEXECUTEINFOA info = {};
		info.cbSize = sizeof(SHELLEXECUTEINFOA);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.lpVerb = "open";
		info.lpFile = filename.c_str();
		info.lpParameters = parameters.c_str();
		ShellExecuteExA(&info);
		WaitForSingleObject(info.hProcess, INFINITE);
	}

	struct UtilsInit
	{
		UtilsInit()
		{
			hInst = GetModuleHandle(nullptr);
#ifdef _MSVC_LANG
			SetProcessDPIAware();
#endif
			screenCx = GetSystemMetrics(SM_CXSCREEN);
			screenCy = GetSystemMetrics(SM_CYSCREEN);

			char buf[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, buf);
			exePath = buf;
		}
	};
	static UtilsInit init;

	OnceFileBuffer::OnceFileBuffer(const std::string &filename)
	{
		auto file = fopen(filename.c_str(), "rb");
		if (!file)
		{
			length = -1;
			return;
		}
		fseek(file, 0, SEEK_END);
		length = ftell(file);
		fseek(file, 0, SEEK_SET);
		data = new char[length + 1];
		fread(data, length, 1, file);
		data[length] = 0;
		fclose(file);
	}

	OnceFileBuffer::~OnceFileBuffer()
	{
		delete data;
	}

	Variable::Variable(What _what, const std::string &_name)
		: what(_what), name(_name)
	{}

	NormalVariable *Variable::toVar()
	{
		return (NormalVariable*)this;
	}

	EnumItem::EnumItem() {}

	EnumItem::EnumItem(const std::string &n, int v)
		:name(n), value(v)
	{}

	EnumVariable *Variable::toEnu()
	{
		return (EnumVariable*)this;
	}

	void Enum::get(const std::string &src, int *dst)
	{
		*dst = 0;

		std::regex pat(R"(([\w_]+))");
		std::string string(src);

		std::smatch sm;
		while (std::regex_search(string, sm, pat))
		{
			auto s = sm[1].str();
			auto found = false;
			for (auto &i : items)
			{
				if (s == i.name)
				{
					*dst |= i.value;
					found = true;
					break;
				}
			}
			assert(found);
			string = sm.suffix();
		}
	}

	EnumVariable::EnumVariable(const std::string &_name, Enum *_pEnum, int *p)
		: Variable(Variable::eEnum, _name), pEnum(_pEnum), _ptr(p)
	{}

	int *EnumVariable::ptr(void *p)
	{
		if (!p) return _ptr;
		return (int*)((LONG_PTR)p + (LONG_PTR)_ptr);
	}

	std::vector<ReflectionBank*> _reflectionBanks;
	std::vector<Enum*> _reflectEnums;

	void ReflectionBank::addE(const std::string &eName, const std::string &name, size_t offset)
	{
		for (auto i = 0; i < _reflectEnums.size(); i++)
		{
			if (_reflectEnums[i]->name == eName)
			{
				auto e = new EnumVariable(name, _reflectEnums[i], (int*)offset);
				reflections.push_back(e);
				break;
			}
		}
	}

	void ReflectionBank::enumertateReflections(void(*callback)(Variable*, int, void*), void *user_data, int offset)
	{
		for (auto r : reflections)
			callback(r, offset, user_data);
		for (auto &p : parents)
			p.first->enumertateReflections(callback, user_data, offset + p.second);
	}

	std::pair<Variable*, int> ReflectionBank::findReflection(const std::string &name, int offset)
	{
		for (auto r : reflections)
		{
			if (r->name == name)
				return std::make_pair(r, offset);
		}
		for (auto &p : parents)
		{
			auto r = p.first->findReflection(name, offset + p.second);
			if (r.first) return r;
		}
		return std::make_pair(nullptr, 0);
	}

	ReflectionBank *addReflectionBank(std::string str)
	{
		auto bank = new ReflectionBank;
		bank->name = str;
		_reflectionBanks.push_back(bank);
		return bank;
	}

	Enum *addReflectEnum(std::string str)
	{
		auto _enum = new Enum;
		_enum->name = str;
		_reflectEnums.push_back(_enum);
		return _enum;
	}

	Attribute::Attribute() {}

	Attribute::Attribute(const std::string &n, const std::string &v)
		:name(n), value(v)
	{}

	Attribute::Attribute(const std::string &n, const char *v) 
		:name(n), value(v)
	{}

	Attribute::Attribute(const std::string &n, char *v) 
		:name(n), value(v)
	{}

	void Attribute::set(const std::type_index &t, void *_v)
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

	void Attribute::get(const std::type_index &t, void *_v)
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

	AttributeTreeNode::AttributeTreeNode(const std::string &_name)
		: name(_name)
	{}

	AttributeTreeNode::~AttributeTreeNode()
	{
		for (auto a : attributes) delete a;
		for (auto c : children) delete c;
	}

	Attribute *AttributeTreeNode::firstAttribute(const std::string &_name)
	{
		for (auto a : attributes)
		{
			if (a->name == _name)
				return a;
		}
		return nullptr;
	}

	AttributeTreeNode *AttributeTreeNode::firstNode(const std::string &_name)
	{
		for (auto c : children)
		{
			if (c->name == _name)
				return c;
		}
		return nullptr;
	}

	void AttributeTreeNode::addAttributes(void *src, ReflectionBank *b)
	{
		ptr = src;
		b->enumertateReflections([](Variable *r, int offset, void *_data) {
			auto n = (AttributeTreeNode*)_data;

			auto a = new Attribute;
			a->name = r->name;

			if (r->what == Variable::eVariable)
			{
				auto v = r->toVar();
				a->set(v->type, v->ptr((void*)((TK_LONG_PTR)n->ptr + offset)));
			}
			else if (r->what == Variable::eEnum)
			{
				auto e = r->toEnu();
				auto v = *e->ptr(n->ptr);

				bool first = true;
				for (int i = 0; i < e->pEnum->items.size(); i++)
				{
					auto &item = e->pEnum->items[i];
					if (v & item.value)
					{
						if (!first)a->value += " ";
						a->value += item.name;
						first = false;
					}
				}
			}

			n->attributes.push_back(a);
		}, this, 0);
	}

	void AttributeTreeNode::obtainFromAttributes(void *dst, ReflectionBank *b)
	{
		ptr = dst;
		for (auto a : attributes)
		{
			auto r = b->findReflection(a->name, 0);
			if (!r.first)
			{
				printf("cannot find \"%s\" reflection from %s", a->name, b->name);
				continue;
			}
			switch (r.first->what)
			{
			case Variable::eVariable:
			{
				auto v = r.first->toVar();
				a->get(v->type, v->ptr((void*)((TK_LONG_PTR)dst + r.second)));
			}
				break;
			case Variable::eEnum:
				r.first->toEnu()->pEnum->get(a->value, r.first->toEnu()->ptr(dst));
				break;
			}
		}
	}

	AttributeTree::AttributeTree(const std::string &_name)
		: AttributeTreeNode(_name)
	{}

	AttributeTree::AttributeTree(const std::string &_name, const std::string &_filename)
		: AttributeTreeNode(_name)
	{
		loadXML(_filename);
	}

	static void _loadXML(rapidxml::xml_node<> *n, AttributeTreeNode *p)
	{
		p->value = n->value();
		for (auto a = n->first_attribute(); a; a = a->next_attribute())
		{
			auto a_ = new Attribute;
			a_->name = a->name();
			a_->value = a->value();
			p->attributes.push_back(a_);
		}

		for (auto nn = n->first_node(); nn; nn = nn->next_sibling())
		{
			auto c = new AttributeTreeNode(nn->name());
			p->children.push_back(c);
			_loadXML(nn, c);
		}
	}

	void AttributeTree::loadXML(const std::string &filename)
	{
		OnceFileBuffer file(filename);
		if (file.length == -1)
		{
			good = false;
			return;
		}

		rapidxml::xml_document<> xmlDoc;
		xmlDoc.parse<0>(file.data);

		auto rootNode = xmlDoc.first_node(name.c_str());
		if (rootNode)
			_loadXML(rootNode, this);
	}

	static void _saveXML(rapidxml::xml_document<> &doc, rapidxml::xml_node<> *n, AttributeTreeNode *p)
	{
		for (auto a : p->attributes)
			n->append_attribute(doc.allocate_attribute(a->name.c_str(), a->value.c_str()));

		for (auto c : p->children)
		{
			auto node = doc.allocate_node(rapidxml::node_element, c->name.c_str());
			n->append_node(node);
			_saveXML(doc, node, c);
		}
	}

	void AttributeTree::saveXML(const std::string &filename)
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

	Observed::~Observed()
	{
		for (auto o : observers)
			o->deadCallback();
	}

	void Observed::addObserver(Observer *o)
	{
		observers.push_back(o);
	}

	void Observed::removeObserver(Observer *o)
	{
		for (auto it = observers.begin(); it != observers.end(); it++)
		{
			if (*it == o)
			{
				observers.erase(it);
				return;
			}
		}
	}
}
