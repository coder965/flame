#include <flame/container/spare_list.h>

namespace tke
{
	SpareList::SpareList(unsigned int _capacity) :
		capacity(_capacity)
	{
		for (int i = 0; i < capacity; i++)
			spare_list.push_back(i);
	}

	int SpareList::get_capacity() const
	{
		return capacity;
	}

	int SpareList::get_size() const
	{
		return map.size();
	}

	int SpareList::add(void *p)
	{
		{
			auto it = map.find(p);
			if (it != map.end())
				return -2;
		}

		if (spare_list.size() == 0)
			return -1;

		{
			auto it = spare_list.begin();
			auto index = *it;
			map[p] = index;
			spare_list.erase(it);
			return index;
		}
	}

	void SpareList::remove(void *p)
	{
		auto it = map.find(p);
		if (it == map.end())
			return;

		auto index = it->second;
		map.erase(it);
		spare_list.push_back(index);
	}

	void SpareList::iterate(const std::function<bool(int index, void *p, bool &remove)> 
		&callback)
	{
		for (auto it = map.begin(); it != map.end(); )
		{
			bool remove = false;
			auto index = it->second;
			auto _continue = callback(index, it->first, remove);
			if (remove)
			{
				it = map.erase(it);
				spare_list.push_back(index);
			}
			else
				it++;
			if (!_continue)
				break;
		}
	}
}
