#pragma once

#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <typeindex>
#include <memory>
#include <filesystem>

#if defined(_WIN64)
typedef __int64 TK_LONG_PTR;
#else
typedef _W64 long TK_LONG_PTR;
#endif

#define TK_ARRAYSIZE(_ARR)      ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define TK_STRUCT_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)

template<class T>
inline std::ifstream& operator&(std::ifstream &file, T &v)
{
	file.read((char*)&v, sizeof(T));
	return file;
}

template<class T>
inline std::ofstream& operator&(std::ofstream &file, T &v)
{
	file.write((char*)&v, sizeof(T));
	return file;
}

inline std::ifstream& operator>(std::ifstream &file, std::string &str)
{
	int size;
	file >> size;
	str.resize(size);
	file.read((char*)str.data(), size);
	return file;
}

inline std::ofstream& operator<(std::ofstream &file, std::string &str)
{
	file << str.size();
	file.write((char*)str.data(), str.size());
	return file;
}

template<size_t s> struct Sizer {};

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

	enum Err
	{
		NoErr,
		ErrInvalidEnum,
		ErrInvalidValue,
		ErrInvalidOperation,
		ErrOutOfMemory,
		ErrContextLost,
		ErrResourceLost
	};

	const char *getErrorString(Err errNum);

	extern void *hInst;
	extern int screenCx;
	extern int screenCy;
	extern std::string exePath;

	struct KeyState
	{
		bool justDown = false;
		bool justUp = false;
		bool pressing = false;
	};

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

	void iterateDirectory(const std::experimental::filesystem::path &filepath, void(*callback)(const std::experimental::filesystem::path &name, bool is_directory));

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
		std::type_index type;
		void *p;

		template<class T>
		NormalVariable(const std::string &_name, T *_ptr)
			: Variable(Variable::eVariable, _name), type(typeid(T)), p(_ptr)
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

	struct Enum
	{
		std::string name;
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
		std::vector<std::pair<ReflectionBank*, int>> parents;
		std::string name;
		std::vector<Variable*> reflections;

		template<class T>
		void addV(const std::string &name, size_t offset)
		{
			auto v = new NormalVariable(name, (T*)offset);
			reflections.push_back(v);
		}

		void addE(const std::string &eName, const std::string &name, size_t offset);
		
		void enumertateReflections(void(*callback)(Variable*, int, void*), void *user_data, int offset);
		std::pair<Variable*, int> findReflection(const std::string &name, int offset);
	};

	ReflectionBank *addReflectionBank(std::string str);
	Enum *addReflectEnum(std::string str);

	struct Attribute
	{
		std::string name;
		std::string value;

		Attribute();
		Attribute(const std::string &, const std::string &);
		Attribute(const std::string &, const char *);
		Attribute(const std::string &, char *);

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
		void *ptr = nullptr;
		std::string name;
		std::string value;
		std::vector<std::unique_ptr<Attribute>> attributes;
		std::vector<std::unique_ptr<AttributeTreeNode>> children;

		AttributeTreeNode(const std::string &_name);
		void add(Attribute *);
		void add(AttributeTreeNode *);
		Attribute *firstAttribute(const std::string &_name);
		AttributeTreeNode *firstNode(const std::string &_name);

		template <class... _Valty>
		inline void addAttribute(_Valty&&... _Val)
		{
			auto a = new Attribute(_Val...);
			add(a);
		}

		void addAttributes(void *src, ReflectionBank *b);
		void obtainFromAttributes(void *dst, ReflectionBank *b);
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

	struct Observer
	{
		virtual void deadCallback() = 0;
	};

	struct Observed
	{
		std::vector<Observer*> observers;

		~Observed();
		void addObserver(Observer *o);
		void removeObserver(Observer *o);
	};
}
