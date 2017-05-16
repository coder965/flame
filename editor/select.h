#ifndef __SELECT__
#define __SELECT__

#include "../src/core/light.h"
#include "../src/core/object.h"
#include "../src/core/model.h"
#include "../src/core/terrain.h"

enum class SelectType
{
	eNull = -1,
	eLight,
	eObject,
	eTerrain,
	eRigidbody,
	eShape,
	eJoint
};

extern SelectType selectType;
extern void *selecting;

inline tke::Light *selectLight()
{
	return (tke::Light*)selecting;
}
inline tke::Object *selectObject()
{
	return (tke::Object*)selecting;
}
inline tke::Rigidbody *selectRigidbody()
{
	return (tke::Rigidbody*)selecting;
}
inline tke::Shape *selectShape()
{
	return (tke::Shape*)selecting;
}
inline tke::Joint *selectJoint()
{
	return (tke::Joint*)selecting;
}
inline tke::Terrain *selectTerrain()
{
	return (tke::Terrain*)selecting;
}

tke::Transformer *selectTransformer();

void *getSelectPtr(SelectType type, int id);

#endif