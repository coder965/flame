#pragma once

#include "../../src/utils.h"
#include "../../src/entity/object.h"

enum ItemType
{
	ItemTypeNull,
	ItemTypeObject,
	ItemTypeLight
};

struct SelectedItem : tke::Observer
{
	ItemType type = ItemTypeNull;
	tke::Observed *ptr = nullptr;

	virtual void deadCallback() override;

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
		switch (type)
		{
		case ItemTypeObject:
			return (tke::Object*)ptr;
			break;
		}
		return nullptr;
	}

	void reset();
	void select(tke::Object *_obj);
};

extern SelectedItem selectedItem;