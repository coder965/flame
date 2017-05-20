#include "select.h"
#include "../src/scene.h"

SelectType selectType = SelectType::eNull;
void *selecting;

tke::Transformer *selectTransformer()
{
	if (selectType == SelectType::eNull) return nullptr;
	return (tke::Transformer*)selecting;
}

void *getSelectPtr(SelectType type, int id)
{
	switch (type)
	{
	case SelectType::eRigidbody:
		return tke::scene->getRigidbody(id);
	case SelectType::eShape:
		return tke::scene->getShape(id);
	case SelectType::eJoint:
		return tke::scene->getJoint(id);
	case SelectType::eLight:
		return tke::scene->getLight(id);
	case SelectType::eObject:
		return tke::scene->getObject(id);
	case SelectType::eTerrain:
		return tke::scene->getTerrain(id);
	}
	return nullptr;
}