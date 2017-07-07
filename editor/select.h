#pragma once

#include "../src/utils.h"
#include "../src/object.h"

enum ItemType
{
	ItemTypeNull,
	ItemTypeObject,
	ItemTypeLight
};

struct SelectedItem : tke::Observer
{
	ItemType type = ItemTypeNull;
	tke::ObservedObject *ptr = nullptr;

	inline explicit operator bool() const noexcept
	{
		return type != ItemTypeNull;
	}

	inline tke::Object *toObject() const
	{
		if (type == ItemTypeObject)
			return (tke::Object*)ptr;
		return nullptr;
	}

	inline tke::Transformer *toTransformer() const
	{
		if (type == ItemTypeNull)
			return nullptr;
		return (tke::Transformer*)ptr;
	}

	void reset();
	void select(tke::Object *_obj);
	virtual void listen(void *sender, tke::NotificationType type, void *newData) override;
};
