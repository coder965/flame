#ifndef __TKE_EDITOR__
#define __TKE_EDITOR__

#include "..\..\core\light.h"
#include "..\..\core\terrain.h"
#include "..\..\core\model.h"
#include "..\..\core\core.h"
#include "select.h"
#include "tool.h"
#include "history.h"

namespace tke
{
	extern SelectType selectType;
	extern void *selecting;
	extern std::vector<History*> histories;
	extern int currentHistory;
	extern Tool *currentTool;
	extern TransformTool transformTool;
	void addHistory(History *history);
	void undo();
	void redo();
	void setTool(Tool *pTool);
	inline Light *selectLight()
	{
		return (Light*)selecting;
	}
	inline Object *selectObject()
	{
		return (Object*)selecting;
	}
	inline Rigidbody *selectRigidbody()
	{
		return (Rigidbody*)selecting;
	}
	inline Shape *selectShape()
	{
		return (Shape*)selecting;
	}
	inline Joint *selectJoint()
	{
		return (Joint*)selecting;
	}
	inline Terrain *selectTerrain()
	{
		return (Terrain*)selecting;
	}
	void select();
	void select(SelectType type, void *ptr);
	inline void select(Light *pLight)
	{
		select(SelectType::eLight, pLight);
	}

	inline void select(Object *pObject)
	{
		select(SelectType::eObject, pObject);
	}

	inline void select(Rigidbody *pRigidbody)
	{
		select(SelectType::eRigidbody, pRigidbody);
	}

	inline void select(Shape *pShape)
	{
		select(SelectType::eShape, pShape);
	}

	inline void select(Joint *pJoint)
	{
		select(SelectType::eJoint, pJoint);
	}

	inline void select(Terrain *pTerrain)
	{
		select(SelectType::eTerrain, pTerrain);
	}
	Transformer *selectTransformer();
	void *getSelectPtr(SelectType type, int id);
	void beginRecordTransformHistory();
	void endRecordTransformHistory();
	void moveTransformer(SelectType selectType, Transformer *pTrans, glm::vec3 coord);
	inline void moveTransformer(Light *pLight, glm::vec3 coord)
	{
		moveTransformer(SelectType::eLight, pLight, coord);
	}
	inline void moveTransformer(Object *pObject, glm::vec3 coord)
	{
		moveTransformer(SelectType::eObject, pObject, coord);
	}
	inline void moveTransformer(Terrain *pTerrain, glm::vec3 coord)
	{
		moveTransformer(SelectType::eTerrain, pTerrain, coord);
	}
	inline void moveTransformer(Rigidbody *pRigidbody, glm::vec3 coord)
	{
		moveTransformer(SelectType::eRigidbody, pRigidbody, coord);
	}
	inline void moveTransformer(Shape *pShape, glm::vec3 coord)
	{
		moveTransformer(SelectType::eShape, pShape, coord);
	}
	inline void moveTransformer(Joint *pJoint, glm::vec3 coord)
	{
		moveTransformer(SelectType::eJoint, pJoint, coord);
	}
	void setTransformerEuler(SelectType selectType, Transformer *pTrans, glm::vec3 euler);
	inline void setTransformerEuler(Light *pLight, glm::vec3 euler)
	{
		setTransformerEuler(SelectType::eLight, pLight, euler);
	}
	inline void setTransformerEuler(Object *pObject, glm::vec3 euler)
	{
		setTransformerEuler(SelectType::eObject, pObject, euler);
	}
	inline void setTransformerEuler(Terrain *pTerrain, glm::vec3 euler)
	{
		setTransformerEuler(SelectType::eTerrain, pTerrain, euler);
	}
	inline void setTransformerEuler(Rigidbody *pRigidbody, glm::vec3 euler)
	{
		setTransformerEuler(SelectType::eRigidbody, pRigidbody, euler);
	}
	inline void setTransformerEuler(Shape *pShape, glm::vec3 euler)
	{
		setTransformerEuler(SelectType::eShape, pShape, euler);
	}
	inline void setTransformerEuler(Joint *pJoint, glm::vec3 euler)
	{
		setTransformerEuler(SelectType::eJoint, pJoint, euler);
	}
	void scaleTransformer(SelectType selectType, Transformer *pTrans, glm::vec3 scale);
	inline void scaleTransformer(Light *pLight, glm::vec3 scale)
	{
		scaleTransformer(SelectType::eLight, pLight, scale);
	}
	inline void scaleTransformer(Object *pObject, glm::vec3 scale)
	{
		scaleTransformer(SelectType::eObject, pObject, scale);
	}
	inline void scaleTransformer(Terrain *pTerrain, glm::vec3 scale)
	{
		scaleTransformer(SelectType::eTerrain, pTerrain, scale);
	}
	inline void scaleTransformer(Rigidbody *pRigidbody, glm::vec3 scale)
	{
		scaleTransformer(SelectType::eRigidbody, pRigidbody, scale);
	}
	inline void scaleTransformer(Shape *pShape, glm::vec3 scale)
	{
		scaleTransformer(SelectType::eShape, pShape, scale);
	}
	inline void scaleTransformer(Joint *pJoint, glm::vec3 scale)
	{
		scaleTransformer(SelectType::eJoint, pJoint, scale);
	}
	void rotateTransformerAxis(SelectType selectType, Transformer *pTrans, Transformer::Axis which, float ang);
	inline void rotateTransformerAxis(Light *pLight, Transformer::Axis which, float ang)
	{
		rotateTransformerAxis(SelectType::eLight, pLight, which, ang);
	}
	inline void rotateTransformerAxis(Object *pObject, Transformer::Axis which, float ang)
	{
		rotateTransformerAxis(SelectType::eObject, pObject, which, ang);
	}
	inline void rotateTransformerAxis(Terrain *pTerrain, Transformer::Axis which, float ang)
	{
		rotateTransformerAxis(SelectType::eTerrain, pTerrain, which, ang);
	}
	inline void rotateTransformerAxis(Rigidbody *pRigidbody, Transformer::Axis which, float ang)
	{
		rotateTransformerAxis(SelectType::eRigidbody, pRigidbody, which, ang);
	}
	inline void rotateTransformerAxis(Shape *pShape, Transformer::Axis which, float ang)
	{
		rotateTransformerAxis(SelectType::eShape, pShape, which, ang);
	}
	inline void rotateTransformerAxis(Joint *pJoint, Transformer::Axis which, float ang)
	{
		rotateTransformerAxis(SelectType::eJoint, pJoint, which, ang);
	}
}

#endif