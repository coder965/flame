#include "select.h"

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

	if (ptr)
		ptr->addObserver(this);
}

void SelectedItem::listen(void *sender, tke::NotificationType type, void *newData)
{
	ptr = (tke::ObservedObject*)newData;
}
