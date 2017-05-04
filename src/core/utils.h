#ifndef __TKE_UTILS__
#define __TKE_UTILS__

#include <memory>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <typeindex>
#include <Windows.h>

inline std::ifstream& operator>>(std::ifstream &file, std::string &str)
{
	int size;
	file >> size;
	str.resize(size);
	file.read((char*)str.data(), size);
	return file;
}

inline std::ofstream& operator<<(std::ofstream &file, std::string &str)
{
	file << str.size();
	file.write((char*)str.data(), str.size());
	return file;
}

template<size_t s> struct Sizer {};


#define PITCH(x) (x % 4 == 0 ? x : 4 - x % 4 + x)

// hash

unsigned int _HASH(char const * str, unsigned int seed);

#define HASH(x) (_HASH(x, 0))

constexpr unsigned int _CHASH(char const * str, unsigned int seed)
{
	return 0 == *str ? seed : _CHASH(str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

template <unsigned int N>
struct EnsureConst
{
	static const unsigned int value = N;
};

#define CHASH(x) (EnsureConst<_CHASH(x, 0)>::value)

namespace tke
{
	enum class Err
	{
		eNoErr,
		eInvalidEnum,
		eInvalidValue,
		eInvalidOperation,
		eOutOfMemory,
		eContextLost,
		eResourceLost
	};

	const char *getErrorString(Err errNum);

	extern HINSTANCE hInst;
	extern int screenCx;
	extern int screenCy;
	extern std::string exePath;

	const char *getClipBoard();
	void setClipBoard(const char *s);
	void saveBitmap24(const std::string &filename, int width, int height, void *data);
	void saveBitmap32(const std::string &filename, int width, int height, void *data);
	void exec(const std::string &filename, const std::string &parameters);

	struct OnceFileBuffer
	{
		int length;
		char *data;
		OnceFileBuffer(const std::string &filename);
		~OnceFileBuffer();
	};

	struct Any
	{
		std::type_index typeIndex;
		void *ptr;

		template<class T>
		Any(T *p)
			: typeIndex(typeid(T)), ptr(p)
		{}

		std::type_index type()
		{
			return typeIndex;
		}
	};

	struct NormalVariable;
	struct EnumVariable;

	struct Variable
	{
		enum What
		{
			eVariable,
			eEnum
		};
		What what;
		std::string name;

		Variable(What _what, const std::string &_name);
		NormalVariable *toVar();
		EnumVariable *toEnu();
	};

	struct NormalVariable : Variable
	{
		Any v;

		template<class T>
		NormalVariable(const std::string &_name, T *ptr)
			: Variable(Variable::eVariable, _name), v(ptr)
		{}

		std::type_index type()
		{
			return v.type();
		}

		template<class T>
		T *ptr(void *p = nullptr)
		{
			if (!p) return (T*)v.ptr;
			return (T*)((int)p + (int)v.ptr);
		}
	};

	struct Enum
	{
		std::vector<std::pair<std::string, int>> items;
	};

	struct EnumVariable : Variable
	{
		Enum *pEnum;
		int *_ptr;

		EnumVariable(const std::string &_name, Enum *_pEnum, int *p);

		int *ptr(void *p = nullptr);
	};

	struct ReflectionBank
	{
		std::vector<Variable*> reflectons;

		template<class T>
		void addV(const std::string &name, size_t offset)
		{
			auto v = new NormalVariable(name, (T*)offset);
			reflectons.push_back(v);
		}

		void addE(const std::string &eName, const std::string &name, size_t offset);
	};

	ReflectionBank *addReflectionBank(std::string str);
	Enum *addReflectEnum(std::string str);

	struct AttributeTreeNode
	{
		std::string name;
		std::vector<std::pair<Variable*, std::string>> atrributes;
		std::vector<AttributeTreeNode*> children;

		AttributeTreeNode(const std::string &_name);
		~AttributeTreeNode();
		AttributeTreeNode *firstNode(const std::string &_name);
		void addAttributes(void *p, ReflectionBank *b);
		void obtainFromAttributes(void *p, ReflectionBank *b);
		void loadXML(const std::string &filename);
		void saveXML(const std::string &filename);
	};

	typedef AttributeTreeNode AttributeTree;

#define REFLECTABLE
#define REFL_BANK static tke::ReflectionBank *b
#define REFLv
#define REFLe

	struct Element
	{
		enum Mark
		{
			eDefault,
			eMarkUp,
			eMarkDown,
			eMarkClear
		};
		Mark mark = eDefault;
	};

	struct Container
	{
		virtual void maintain(int row) = 0;
	};

	// one operation a time

	template <class T>
	void maintainVector(std::vector<T> &v)
	{
		for (auto i = 0; i < v.size(); i++)
		{
			switch (v[i]->mark)
			{
			case Element::eMarkUp:
				if (i > 0) std::swap(v[i], v[i - 1]);
				v[i]->mark = Element::eDefault;
				return;
			case Element::eMarkDown:
				if (i < v.size() - 1) std::swap(v[i], v[i + 1]);
				v[i]->mark = Element::eDefault;
				return;
			case Element::eMarkClear:
				v.erase(v.begin() + i);
				return;
			}
		}
	}

	template <class T>
	void maintainList(std::list<T> &l)
	{
		for (auto it = l.begin(); it != l.end(); it++)
		{
			switch (it->mark)
			{
			case Element::eMarkUp:
				if (it != l.begin()) l.splice(it, l, --it);
				it->mark = Element::eDefault;
				return;
			case Element::eMarkDown:
				if (it != l.end()) l.splice(it, l, ++it);
				it->mark = Element::eDefault;
				return;
			case Element::eMarkClear:
				l.erase(it);
				return;
			}
		}
	}
}

#endif
