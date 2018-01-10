#include "select.h"

static std::string select_filename;

Select selected;

void Select::reset()
{
	if (type == SelectTypeNode)
	{
		for (auto f : followings)
			f->remove_follower(this);
		followings.clear();
	}
	type = SelectTypeNull;
	select_filename.clear();
}

void Select::operator=(tke::Node *n)
{
	if (type == SelectTypeNode)
	{
		for (auto f : followings)
			f->remove_follower(this);
		followings.clear();
	}
	type = SelectTypeNode;
	select_filename.clear();
	follow_to(n);
}

void Select::operator=(const std::string &s)
{
	if (type == SelectTypeNode)
	{
		for (auto f : followings)
			f->remove_follower(this);
		followings.clear();
	}
	type = SelectTypeFile;
	select_filename = s;
}

tke::Node *Select::get_node()
{
	return type == SelectTypeNode ? (tke::Node*)followings[0] : nullptr;
}

const std::string &Select::get_filename()
{
	return type == SelectTypeFile ? select_filename : "";
}
