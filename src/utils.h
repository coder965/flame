#pragma once

#include <vector>
#include <list>
#include <string>
#include <typeindex>
#include <memory>
#include <chrono>

#if defined(_WIN64)
typedef __int64 TK_LONG_PTR;
#else
typedef _W64 long TK_LONG_PTR;
#endif

#define TK_ARRAYSIZE(_ARR)      ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define TK_DERIVE_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)
#define TK_LOW(I) ((I) & 0xffff)
#define TK_HIGH(I) ((I) >> 16)
#define TK_MAKEINT(H, L) ((L) | ((H) << 16))

template<size_t s> struct Sizer {};

namespace tke
{
	typedef void(*PF_EVENT0)();
	typedef void(*PF_EVENT1)(int);
	typedef void(*PF_EVENT2)(int, int);

	inline long long now_time_ms()
	{
		return std::chrono::system_clock::now().time_since_epoch().count() / 10000;
	}

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

	struct Attribute
	{
		std::string name;
		std::string value;

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

			set(v);
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
		Attribute *newAttribute();
		Attribute *newAttribute(const std::string &, const std::string &);
		Attribute *newAttribute(const std::string &, const char *);
		Attribute *newAttribute(const std::string &, char *);

		template<class T>
		Attribute *newAttribute(const std::string &n, T *v)
		{
			auto a = newAttribute();
			a->set(v);
			return a;
		}

		AttributeTreeNode *newNode(const std::string &_name);
		Attribute *firstAttribute(const std::string &_name);
		AttributeTreeNode *firstNode(const std::string &_name);

		template <class... _Valty>
		inline void addAttribute(_Valty&&... _Val)
		{
			newAttribute(_Val...);
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
#define IMPL(INIT) extern

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
