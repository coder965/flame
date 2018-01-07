#include "select.h"

static std::string select_filename;

Select selected;

void Select::reset()
{
	type = SelectTypeNull;
	node.reset();
	select_filename.clear();
}

void Select::operator=(std::shared_ptr<tke::Node> n)
{
	type = SelectTypeNode;
	select_filename.clear();
	node = n;
}

void Select::operator=(const std::string &s)
{
	type = SelectTypeFile;
	node.reset();
	select_filename = s;
}

tke::Node *Select::get_node()
{
	if (type != SelectTypeNode)
		return nullptr;
	auto s = node.lock();
	if (s)
		return s.get();
	reset();
	return nullptr;
}

const std::string &Select::get_filename()
{
	if (type != SelectTypeFile)
		return "";
	return select_filename;
}
