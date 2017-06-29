#pragma once

#include <memory>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <typeindex>
#include <Windows.h>

template<class T>
inline std::ifstream& operator>>(std::ifstream &file, T &v)
{
	file.read((char*)&v, sizeof(T));
	return file;
}

template<class T>
inline std::ofstream& operator<<(std::ofstream &file, T &v)
{
	file.write((char*)&v, sizeof(T));
	return file;
}

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
	int lineNumber(const char *str);

	struct CriticalSection
	{
		CRITICAL_SECTION v;
		inline CriticalSection() 
		{
			InitializeCriticalSection(&v);
		}
		inline void lock()
		{
			EnterCriticalSection(&v);
		}
		inline void unlock()
		{
			LeaveCriticalSection(&v);
		}
	};

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
	void setClipBoard(const std::string &);

	std::string translate(int srcCP, int dstCP, const std::string &src);
	std::string japaneseToChinese(const std::string &src);

	void saveBitmap24(const std::string &filename, int width, int height, void *data);
	void saveBitmap32(const std::string &filename, int width, int height, void *data);

	void exec(const std::string &filename, const std::string &parameters);

	struct OnceFileBuffer
	{
		int length = 0;
		char *data = nullptr;
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

		std::type_index type();
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

		std::type_index type();

		void *ptr(void *p = nullptr);

		template<class T>
		T *ptr(void *p = nullptr)
		{
			return (T*)ptr(p);
		}
	};

	struct EnumItem
	{
		std::string name;
		int value;

		EnumItem();
		EnumItem(const std::string &, int);
	};

	struct Enum
	{
		std::vector<EnumItem> items;
		void get(const std::string &src, int *dst);
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

	struct Attribute
	{
		std::string name;
		std::string value;

		Attribute();
		Attribute(const std::string &, const std::string &);

		template<class T>
		Attribute(const std::string &n, T *v)
			:name(n)
		{
			set(v);
		}

		void set(const std::type_index &t, void *v);

		template<class T>
		inline void set(T *v)
		{
			set(typeid(T), v);
		}

		template<class T>
		inline void set(const std::string &n, T *v)
		{
			name = n;

			set(t);
		}

		void get(const std::type_index &t, void *v);

		template<class T>
		inline void get(T *v)
		{
			get(typeid(T), v);
		}
	};

	struct AttributeTreeNode
	{
		std::string name;
		std::vector<Attribute*> attributes;
		std::vector<AttributeTreeNode*> children;

		AttributeTreeNode(const std::string &_name);
		~AttributeTreeNode();
		Attribute *firstAttribute(const std::string &_name);
		AttributeTreeNode *firstNode(const std::string &_name);

		template <class... _Valty>
		inline void addAttribute(_Valty&&... _Val)
		{
			auto a = new Attribute(_Val...);
			attributes.push_back(a);
		}

		void addAttributes(void *p, ReflectionBank *b);
		void obtainFromAttributes(void *p, ReflectionBank *b);
	};

	struct AttributeTree : AttributeTreeNode
	{
		bool good = true;

		AttributeTree(const std::string &_name);
		AttributeTree(const std::string &_name, const std::string &_filename);
		void loadXML(const std::string &filename);
		void saveXML(const std::string &filename);
	};

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

	enum NotificationType
	{
		NotificationTypeChange,
		NotificationTypeRefresh
	};

	struct Observer
	{
		virtual void listen(void *sender, NotificationType type, void *newData) = 0;
	};

	struct ObservedObject
	{
		std::vector<Observer*> observers;

		~ObservedObject();
		void addObserver(Observer*);
		void removeObserver(Observer*);
	};
}
