#pragma once

#include <memory>
#include <string>

#include "../entity/node.h"

enum SelectType
{
	SelectTypeNull,
	SelectTypeNode,
	SelectTypeFile
};

struct Select
{
	SelectType type = SelectTypeNull;
	std::weak_ptr<tke::Node> node;

	void reset();
	void operator=(std::shared_ptr<tke::Node> n);
	void operator=(const std::string &s);
	tke::Node *get_node();
	const std::string &get_filename();
};

extern Select selected;