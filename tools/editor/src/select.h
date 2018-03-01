#pragma once

#include <memory>
#include <string>

#include <flame/engine/object.h>
#include <flame/entity/node.h>

enum SelectType
{
	SelectTypeNull,
	SelectTypeNode,
	SelectTypeFile
};

struct Select : flame::Object
{
	SelectType type = SelectTypeNull;

	void reset();
	void operator=(flame::Node *n);
	void operator=(const std::string &s);
	flame::Node *get_node();
	std::string get_filename();
};

extern Select selected;