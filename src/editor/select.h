#pragma once

#include <memory>
#include <string>

#include "../_object.h"
#include "../entity/node.h"

enum SelectType
{
	SelectTypeNull,
	SelectTypeNode,
	SelectTypeFile
};

struct Select : tke::_Object
{
	SelectType type = SelectTypeNull;

	void reset();
	void operator=(tke::Node *n);
	void operator=(const std::string &s);
	tke::Node *get_node();
	const std::string &get_filename();
	virtual void on_message(tke::_Object*, tke::Message) override {}
};

extern Select selected;