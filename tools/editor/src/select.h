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

struct Select : tke::Object
{
	SelectType type = SelectTypeNull;

	void reset();
	void operator=(tke::Node *n);
	void operator=(const std::string &s);
	tke::Node *get_node();
	std::string get_filename();
};

extern Select selected;