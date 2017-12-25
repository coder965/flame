#pragma once

#include <string>
#include <typeindex>
#include <vector>

#include "define.h"

#define REFLECTABLE
#define REFL_BANK static tke::ReflectionBank *b
#define REFLv
#define REFLe
#define IMPL(INIT) extern

namespace tke
{
	struct Variable;
	struct Enum;

	struct Reflection
	{
		enum What
		{
			eVariable,
			eEnum
		};
		What what;
		std::string name;

		Reflection(What _what, const std::string &_name);
		Variable *toVar();
		Enum *toEnu();
	};

	struct Variable : Reflection
	{
		std::type_index type;
		void *p;

		template<class T>
		Variable(const std::string &_name, T *_ptr)
			: Reflection(Reflection::eVariable, _name), type(typeid(T)), p(_ptr)
		{}

		void *ptr(void *base = nullptr)
		{
			return (void*)((TK_LONG_PTR)base + (TK_LONG_PTR)p);
		}

		template<class T>
		T *ptr(void *_p = nullptr)
		{
			return (T*)ptr(_p);
		}
	};

	struct EnumItem
	{
		std::string name;
		int value;

		EnumItem();
		EnumItem(const std::string &, int);
	};

	struct EnumType
	{
		std::string name;
		std::vector<EnumItem> items;
		void get(const std::string &src, int *dst);
	};

	struct Enum : Reflection
	{
		EnumType *pEnum;
		int *_ptr;

		Enum(const std::string &_name, EnumType *_pEnum, int *p);
		int *ptr(void *p = nullptr);
	};

	struct ReflectionBank
	{
		std::vector<std::pair<ReflectionBank*, int>> parents;
		std::string name;
		std::vector<Reflection*> reflections;

		template<class T>
		void addV(const std::string &name, size_t offset)
		{
			auto v = new Variable(name, (T*)offset);
			reflections.push_back(v);
		}

		void addE(const std::string &eName, const std::string &name, size_t offset);

		void enumertateReflections(void(*callback)(Reflection*, int, void*), void *user_data, int offset);
		std::pair<Reflection*, int> findReflection(const std::string &name, int offset);
	};

	ReflectionBank *addReflectionBank(std::string str);
	EnumType *addReflectEnumType(std::string str);

}
