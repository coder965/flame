#include <assert.h>
#include <regex>

#include "refl.h"

namespace tke
{

	Reflection::Reflection(What _what, const std::string &_name)
		: what(_what), name(_name)
	{}

	Variable *Reflection::toVar()
	{
		return (Variable*)this;
	}

	EnumItem::EnumItem() {}

	EnumItem::EnumItem(const std::string &n, int v)
		:name(n), value(v)
	{}

	Enum *Reflection::toEnu()
	{
		return (Enum*)this;
	}

	void EnumType::get(const std::string &src, int *dst)
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

	Enum::Enum(const std::string &_name, EnumType *_pEnum, int *p)
		: Reflection(Reflection::eEnum, _name), pEnum(_pEnum), _ptr(p)
	{}

	int *Enum::ptr(void *p)
	{
		if (!p) return _ptr;
		return (int*)((TK_LONG_PTR)p + (TK_LONG_PTR)_ptr);
	}

	std::vector<ReflectionBank*> _reflectionBanks;
	std::vector<EnumType*> _reflectEnums;

	void ReflectionBank::addE(const std::string &eName, const std::string &name, size_t offset)
	{
		for (auto reflectEnum : _reflectEnums)
		{
			if (reflectEnum->name == eName)
			{
				auto e = new Enum(name, reflectEnum, (int*)offset);
				reflections.push_back(e);
				return;
			}
		}
		assert(0);
	}

	void ReflectionBank::enumertateReflections(void(*callback)(Reflection*, int, void*), void *user_data, int offset)
	{
		for (auto r : reflections)
			callback(r, offset, user_data);
		for (auto &p : parents)
			p.first->enumertateReflections(callback, user_data, offset + p.second);
	}

	std::pair<Reflection*, int> ReflectionBank::findReflection(const std::string &name, int offset)
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

	EnumType *addReflectEnumType(std::string str)
	{
		auto _enum = new EnumType;
		_enum->name = str;
		_reflectEnums.push_back(_enum);
		return _enum;
	}

}
