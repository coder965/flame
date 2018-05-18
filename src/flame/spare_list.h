//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <map>
#include <vector>
#include <list>
#include <functional>

namespace flame
{
	class SpareList
	{
	protected:
		unsigned int capacity;
		std::map<void*, int> map;
		std::list<int> spare_list;
	public:
		SpareList(unsigned int _capacity) :
			capacity(_capacity)
		{
			for (int i = 0; i < capacity; i++)
				spare_list.push_back(i);
		}

		int get_capacity() const
		{
			return capacity;
		}

		int get_size() const
		{
			return map.size();
		}

		int add(void *p)
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

		void remove(void *p)
		{
			auto it = map.find(p);
			if (it == map.end())
				return;

			auto index = it->second;
			map.erase(it);
			spare_list.push_back(index);
		}

		void iterate(const std::function<bool(int index, void *p, bool &remove)> 
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
	};
}
