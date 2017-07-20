#include "select.h"

void SelectedItem::deadCallback()
{
	ptr = nullptr;
	type = ItemTypeNull;
}

void SelectedItem::reset()
{
	type = ItemTypeNull;
	ptr = nullptr;
}

void SelectedItem::select(tke::Object *_obj)
{
	if (_obj == ptr) return;

	if (ptr)
		ptr->removeObserver(this);

	type = ItemTypeObject;
	ptr = _obj;
	ptr->addObserver(this);
}
