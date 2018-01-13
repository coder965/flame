#pragma once

#include <map>
#include <vector>
#include <list>
#include <functional>

namespace tke
{
	class SpareList
	{
	protected:
		unsigned int capacity;
		std::map<void*, int> map;
		std::list<int> spare_list;
	public:
		SpareList(unsigned int _capacity);

		int get_capacity() const;
		int get_size() const;

		int add(void *p);
		void remove(void *p);

		void iterate(const std::function<void(int index, void *p, bool &remove)> 
			&callback);
	};
}
