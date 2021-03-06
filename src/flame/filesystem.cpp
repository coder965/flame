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

#include <algorithm>

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include <flame/filesystem.h>

namespace flame
{
	// THESE TWO FUNCTIONS ARE SEALED SINCE 2018-01-11

	//void XMLNode::addAttributes(void *src, ReflectionBank *b)
	//{
	//	ptr = src;
	//	b->enumertateReflections([](Reflection *r, int offset, void *_data) {
	//		auto n = (XMLNode*)_data;

	//		auto a = n->newAttribute();
	//		a->name = r->name;

	//		if (r->what == Reflection::eVariable)
	//		{
	//			auto v = r->toVar();
	//			a->set(v->type, v->ptr((void*)((TK_LONG_PTR)n->ptr + offset)));
	//		}
	//		else if (r->what == Reflection::eEnum)
	//		{
	//			auto e = r->toEnu();
	//			auto v = *e->ptr(n->ptr);

	//			bool first = true;
	//			for (int i = 0; i < e->pEnum->items.size(); i++)
	//			{
	//				auto &item = e->pEnum->items[i];
	//				if (v & item.value)
	//				{
	//					if (!first)a->value += " ";
	//					a->value += item.name;
	//					first = false;
	//				}
	//			}
	//		}
	//	}, this, 0);
	//}

	//void XMLNode::obtainFromAttributes(void *dst, ReflectionBank *b)
	//{
	//	ptr = dst;
	//	for (auto &a : attributes)
	//	{
	//		auto r = b->findReflection(a->name, 0);
	//		if (!r.first)
	//		{
	//			printf("cannot find \"%s\" reflection from %s\n", a->name.c_str(), b->name.c_str());
	//			continue;
	//		}
	//		switch (r.first->what)
	//		{
	//		case Reflection::eVariable:
	//		{
	//			auto v = r.first->toVar();
	//			a->get(v->type, v->ptr((void*)((TK_LONG_PTR)dst + r.second)));
	//			break;
	//		}
	//		case Reflection::eEnum:
	//			r.first->toEnu()->pEnum->get(a->value, r.first->toEnu()->ptr(dst));
	//			break;
	//		}
	//	}
	//}

	static void _load_XML(rapidxml::xml_node<> *src, XMLNode *dst)
	{
		dst->content = src->value();
		for (auto a = src->first_attribute(); a; a = a->next_attribute())
			dst->attributes.emplace_back(new XMLAttribute(a->name(), std::string(a->value())));

		for (auto n = src->first_node(); n; n = n->next_sibling())
		{
			auto c = new XMLNode(n->name());
			dst->children.emplace_back(c);
			_load_XML(n, c);
		}
	}

	XMLDoc *load_xml(const std::string &_name, const std::string &filename)
	{
		auto content = get_file_content(filename);
		if (!content.first)
			return nullptr;

		auto doc = new XMLDoc(_name);

		rapidxml::xml_document<> xml_doc;
		xml_doc.parse<0>(content.first.get());

		auto rootNode = xml_doc.first_node(_name.c_str());
		if (rootNode)
			_load_XML(rootNode, doc);

		return doc;
	}

	static void _save_XML(rapidxml::xml_document<> &doc, rapidxml::xml_node<> *n, XMLNode *p)
	{
		for (auto &a : p->attributes)
			n->append_attribute(doc.allocate_attribute(a->name.c_str(), a->value.c_str()));

		for (auto &c : p->children)
		{
			auto node = doc.allocate_node(rapidxml::node_element, c->name.c_str());
			n->append_node(node);
			_save_XML(doc, node, c.get());
		}
	}

	void save_xml(XMLDoc *doc, const std::string &filename)
	{
		rapidxml::xml_document<> xml_doc;
		auto root_node = xml_doc.allocate_node(rapidxml::node_element, doc->name.c_str());
		xml_doc.append_node(root_node);
		_save_XML(xml_doc, root_node, doc);

		std::string str;
		rapidxml::print(std::back_inserter(str), xml_doc);

		std::ofstream file(filename);
		file.write(str.data(), str.size());
	}

	void release_xml(XMLDoc *doc)
	{
		delete doc;
	}
}
