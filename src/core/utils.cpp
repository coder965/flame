#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <regex>

#include "..\..\..\rapidxml-1.13\rapidxml.hpp"
#include "..\..\..\rapidxml-1.13\rapidxml_utils.hpp"
#include "..\..\..\rapidxml-1.13\rapidxml_print.hpp"

#include "utils.h"

// hash

unsigned int _HASH(char const * str, unsigned int seed)
{
	return 0 == *str ? seed : _HASH(str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

namespace tke
{
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

	HINSTANCE hInst;
	int screenCx;
	int screenCy;
	std::string exePath;

	const char *getClipBoard()
	{
		static char *str = nullptr;
		delete[]str;
		OpenClipboard(NULL);
		auto hClipMemory = ::GetClipboardData(CF_TEXT);
		auto dwLength = GlobalSize(hClipMemory);
		auto lpClipMemory = (LPBYTE)GlobalLock(hClipMemory);
		str = new char[dwLength + 1];
		strcpy(str, (char*)lpClipMemory);
		GlobalUnlock(hClipMemory);
		CloseClipboard();
		return str;
	}

	void setClipBoard(const char *s)
	{
		auto dwLength = strlen(s);
		auto hGlobalMemory = GlobalAlloc(GHND, dwLength + 1);
		auto lpGlobalMemory = (LPBYTE)GlobalLock(hGlobalMemory);
		strcpy((char*)lpGlobalMemory, s);
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hGlobalMemory);
		CloseClipboard();
	}

	void japaneseToChinese(char *str, int len)
	{
		wchar_t *wbuf = new wchar_t[len];
		MultiByteToWideChar(932, 0, str, -1, wbuf, len);
		WideCharToMultiByte(936, 0, wbuf, -1, str, len, NULL, false);
		delete[]wbuf;
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
		SHELLEXECUTEINFOA ShExecInfo = {};
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = "open";
		ShExecInfo.lpFile = filename.c_str();
		ShExecInfo.lpParameters = parameters.c_str();
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_HIDE;
		ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&ShExecInfo);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
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
		assert(file);
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

	Reflection::Reflection(What _what, const std::string &_name)
		: what(_what), name(_name)
	{}

	VariableReflection *Reflection::toVar()
	{
		return (VariableReflection*)this;
	}

	EnumReflection *Reflection::toEnu()
	{
		return (EnumReflection*)this;
	}

	EnumReflection::EnumReflection(const std::string &_name, Enum *_pEnum, int *p)
		: Reflection(Reflection::eEnum, _name), pEnum(_pEnum), _ptr(p)
	{}

	int *EnumReflection::ptr(void *p)
	{
		if (!p) return _ptr;
		return (int*)((int)p + (int)_ptr);
	}

	std::vector<std::pair<std::string, ReflectionBank*>> _reflectionBanks;
	std::vector<std::pair<std::string, Enum*>> _reflectEnums;

	void ReflectionBank::addE(const std::string &eName, const std::string &name, size_t offset)
	{
		for (auto i = 0; i < _reflectEnums.size(); i++)
		{
			if (_reflectEnums[i].first == eName)
			{
				auto e = new EnumReflection(name, _reflectEnums[i].second, (int*)offset);
				reflectons.push_back(e);
				break;
			}
		}
	}

	ReflectionBank *addReflectionBank(std::string str)
	{
		auto bank = new ReflectionBank;
		_reflectionBanks.emplace_back(str, bank);
		return bank;
	}

	Enum *addReflectEnum(std::string str)
	{
		auto _enum = new Enum;
		_reflectEnums.emplace_back(str, _enum);
		return _enum;
	}

	AttributeTreeNode::AttributeTreeNode(const std::string &_name)
		: name(_name)
	{}

	AttributeTreeNode::~AttributeTreeNode()
	{
		for (auto c : children) delete c;
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

	void AttributeTreeNode::addAttributes(void *p, ReflectionBank *b)
	{
		for (auto r : b->reflectons)
		{
			if (r->what == Reflection::eVariable)
			{
				auto v = (VariableReflection*)r;
				if (v->type() == typeid(std::string))
				{
					auto _v = new VariableReflection(v->name, v->ptr<std::string>(p));
					atrributes.emplace_back(_v, std::string());
				}
				else if (v->type() == typeid(int))
				{
					auto _v = new VariableReflection(v->name, v->ptr<int>(p));
					atrributes.emplace_back(_v, std::string());
				}
				else if (v->type() == typeid(float))
				{
					auto _v = new VariableReflection(v->name, v->ptr<float>(p));
					atrributes.emplace_back(_v, std::string());
				}
				else if (v->type() == typeid(bool))
				{
					auto _v = new VariableReflection(v->name, v->ptr<bool>(p));
					atrributes.emplace_back(_v, std::string());
				}
			}
			else if (r->what == Reflection::eEnum)
			{
				auto e = (EnumReflection*)r;
				auto _e = new EnumReflection(e->name, e->pEnum, e->ptr(p));
				atrributes.emplace_back(_e, std::string());
			}
		}
	}

	static void _obtainVarFromAttributes(AttributeTreeNode *n, void *p, VariableReflection *v)
	{
		for (auto &a : n->atrributes)
		{
			if (a.first->name == v->name)
			{
				if (v->type() == typeid(std::string))
					*(v->ptr<std::string>(p)) = a.second;
				else if (v->type() == typeid(int))
					*(v->ptr<int>(p)) = std::stoi(a.second);
				else if (v->type() == typeid(float))
					*(v->ptr<float>(p)) = std::stof(a.second);
				else if (v->type() == typeid(bool))
				{
					if (a.second == "true")
						*(v->ptr<bool>(p)) = true;
					else if (a.second == "false")
						*(v->ptr<bool>(p)) = false;
				}
				return;
			}
		}
	}

	static void _obtainEnuFromAttributes(AttributeTreeNode *n, void *p, EnumReflection *e)
	{
		for (auto &a : n->atrributes)
		{
			if (a.first->name == e->name)
			{
				*e->ptr(p) = 0;

				std::regex pat(R"(\w+)");
				std::string string(a.second);

				std::smatch sm;
				while (std::regex_search(string, sm, pat))
				{
					auto s = sm[0].str();
					for (auto &i : e->pEnum->items)
					{
						if (s.compare(i.first) == 0)
						{
							*e->ptr(p) |= i.second;
							break;
						}
					}
					string = sm.suffix();
				}
				return;
			}
		}
	}

	void AttributeTreeNode::obtainFromAttributes(void *p, ReflectionBank *b)
	{
		for (auto r : b->reflectons)
		{
			if (r->what == Reflection::eVariable)
			{
				auto v = (VariableReflection*)r;
				_obtainVarFromAttributes(this, p, v);
			}
			else if (r->what == Reflection::eEnum)
			{
				auto e = (EnumReflection*)r;
				_obtainEnuFromAttributes(this, p, e);
			}
		}
	}

	static void _loadXML(rapidxml::xml_node<> *n, AttributeTreeNode *p)
	{
		for (auto a = n->first_attribute(); a; a = a->next_attribute())
		{
			auto v = new VariableReflection(a->name(), (std::string*)0);
			p->atrributes.emplace_back(v, std::string(a->value()));
		}

		for (auto nn = n->first_node(); nn; nn = nn->next_sibling())
		{
			auto c = new AttributeTreeNode(nn->name());
			p->children.push_back(c);
			_loadXML(nn, c);
		}
	}

	void AttributeTreeNode::loadXML(const std::string &filename)
	{
		OnceFileBuffer file(filename);
		rapidxml::xml_document<> xmlDoc;
		xmlDoc.parse<0>(file.data);

		auto rootNode = xmlDoc.first_node(name.c_str());
		if (rootNode)
			_loadXML(rootNode, this);
	}

	static void _saveXML(rapidxml::xml_document<> &doc, rapidxml::xml_node<> *n, AttributeTreeNode *p)
	{
		for (auto &a : p->atrributes)
		{
			if (a.first->what == Reflection::eVariable)
			{
				auto v = a.first->toVar();
				if (v->type() == typeid(std::string))
				{
					n->append_attribute(doc.allocate_attribute(v->name.c_str(), (v->ptr<std::string>(p))->c_str()));
				}
				else if (v->type() == typeid(int))
				{
					a.second = std::to_string(*(v->ptr<int>(p)));
					n->append_attribute(doc.allocate_attribute(v->name.c_str(), a.second.c_str()));
				}
				else if (v->type() == typeid(float))
				{
					a.second = std::to_string(*(v->ptr<float>(p)));
					n->append_attribute(doc.allocate_attribute(v->name.c_str(), a.second.c_str()));
				}
				else if (v->type() == typeid(bool))
				{
					n->append_attribute(doc.allocate_attribute(v->name.c_str(), (*(v->ptr<bool>(p))) ? "true" : "false"));
				}
			}
			else if (a.first->what == Reflection::eEnum)
			{
				auto e = a.first->toEnu();
				auto v = *e->ptr(p);

				a.second.clear();
				for (auto &i : e->pEnum->items)
				{
					if (v & i.second)
					{
						a.second += i.first;
						a.second += " ";
					}
				}
				n->append_attribute(doc.allocate_attribute(e->name.c_str(), a.second.c_str()));
			}
		}

		for (auto c : p->children)
		{
			auto node = doc.allocate_node(rapidxml::node_element, c->name.c_str());
			doc.append_node(node);
			_saveXML(doc, node, c);
		}
	}

	void AttributeTreeNode::saveXML(const std::string &filename)
	{
		rapidxml::xml_document<> xmlDoc;
		auto rootNode = xmlDoc.allocate_node(rapidxml::node_element, name.c_str());
		xmlDoc.append_node(rootNode);
		_saveXML(xmlDoc, rootNode, this);

		std::string str;
		rapidxml::print(std::back_inserter(str), xmlDoc);

		std::ofstream file(filename);
		file << str;
	}
}
