#pragma once

#include "../src/utils.h"
#include "../src/entity.h"

enum ItemType
{
	ItemTypeNull,
	ItemTypeObject,
	ItemTypeLight
};

struct SelectedItem
{
	ItemType type = ItemTypeNull;
	void *ptr = nullptr;

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
};
