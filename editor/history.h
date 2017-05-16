#ifndef __HISTORY__
#define __HISTORY__

#include "..\src\core\object.h"
#include "select.h"

struct History
{
	enum class Type
	{
		eNull,
		eSelection,
		eTransform,
		eLightCreation,
		eObjectCreation
	};
	enum class Operate
	{
		eUndo,
		eRedo
	};

	Type type;
	virtual void operate(Operate op) = 0;
};

struct SelectionHistory : History
{
	SelectType m_selectType;
	int m_id;

	SelectionHistory();
	virtual void operate(Operate op);
};

struct TransformHistory : History
{
	SelectType m_selectType;
	int m_selectID;
	tke::Transformer::Type m_transformType;
	glm::vec3 m_transformValue;

	TransformHistory(SelectType selectType, int selectID, tke::Transformer::Type transformType, glm::vec3 transformValue);
	virtual void operate(Operate op);
};

struct ObjectCreationHistory : History
{
	enum class CreationType
	{
		eCreate,
		eDelete
	}creationType;

	int id;
	glm::vec3 coord;
	glm::vec3 euler;
	glm::vec3 scale;
	tke::Model *pModel;
	bool phyx;
	tke::Object::MoveType moveType;
	tke::Object::UpMethod upMethod;
	float baseForwardAng;
	float frontSpeed, backSpeed, leftSpeed, rightSpeed, upSpeed, downSpeed;
	glm::vec2 turnSpeed;

	ObjectCreationHistory(tke::Object *pObject, CreationType _type);
	virtual void operate(Operate);
};

extern std::vector<History*> histories;
extern int currentHistory;

void addHistory(History *history);

void undo();

void redo();

void select();

void select(SelectType type, void *ptr);

inline void select(tke::Light *pLight)
{
	select(SelectType::eLight, pLight);
}

inline void select(tke::Object *pObject)
{
	select(SelectType::eObject, pObject);
}

inline void select(tke::Rigidbody *pRigidbody)
{
	select(SelectType::eRigidbody, pRigidbody);
}

inline void select(tke::Shape *pShape)
{
	select(SelectType::eShape, pShape);
}

inline void select(tke::Joint *pJoint)
{
	select(SelectType::eJoint, pJoint);
}

inline void select(tke::Terrain *pTerrain)
{
	select(SelectType::eTerrain, pTerrain);
}

static auto leftTransformerHistory = false;
static auto leftTransformerHistoryFirstHistory = false;
static SelectType leftTransformerHistorySelectType;
static int leftTransformerHistorySelectID;
static tke::Transformer::Type leftTransformerHistoryType;
static glm::vec3 leftTransformerHistoryOriginalValue;
static glm::vec3 leftTransformerHistoryValue;

void beginRecordTransformHistory();

void endRecordTransformHistory();

void moveTransformer(SelectType selectType, tke::Transformer *pTrans, glm::vec3 coord);

inline void moveTransformer(tke::Light *pLight, glm::vec3 coord)
{
	moveTransformer(SelectType::eLight, pLight, coord);
}
inline void moveTransformer(tke::Object *pObject, glm::vec3 coord)
{
	moveTransformer(SelectType::eObject, pObject, coord);
}
inline void moveTransformer(tke::Terrain *pTerrain, glm::vec3 coord)
{
	moveTransformer(SelectType::eTerrain, pTerrain, coord);
}
inline void moveTransformer(tke::Rigidbody *pRigidbody, glm::vec3 coord)
{
	moveTransformer(SelectType::eRigidbody, pRigidbody, coord);
}
inline void moveTransformer(tke::Shape *pShape, glm::vec3 coord)
{
	moveTransformer(SelectType::eShape, pShape, coord);
}
inline void moveTransformer(tke::Joint *pJoint, glm::vec3 coord)
{
	moveTransformer(SelectType::eJoint, pJoint, coord);
}

void setTransformerEuler(SelectType selectType, tke::Transformer *pTrans, glm::vec3 euler);

inline void setTransformerEuler(tke::Light *pLight, glm::vec3 euler)
{
	setTransformerEuler(SelectType::eLight, pLight, euler);
}
inline void setTransformerEuler(tke::Object *pObject, glm::vec3 euler)
{
	setTransformerEuler(SelectType::eObject, pObject, euler);
}
inline void setTransformerEuler(tke::Terrain *pTerrain, glm::vec3 euler)
{
	setTransformerEuler(SelectType::eTerrain, pTerrain, euler);
}
inline void setTransformerEuler(tke::Rigidbody *pRigidbody, glm::vec3 euler)
{
	setTransformerEuler(SelectType::eRigidbody, pRigidbody, euler);
}
inline void setTransformerEuler(tke::Shape *pShape, glm::vec3 euler)
{
	setTransformerEuler(SelectType::eShape, pShape, euler);
}
inline void setTransformerEuler(tke::Joint *pJoint, glm::vec3 euler)
{
	setTransformerEuler(SelectType::eJoint, pJoint, euler);
}

void scaleTransformer(SelectType selectType, tke::Transformer *pTrans, glm::vec3 scale);

inline void scaleTransformer(tke::Light *pLight, glm::vec3 scale)
{
	scaleTransformer(SelectType::eLight, pLight, scale);
}
inline void scaleTransformer(tke::Object *pObject, glm::vec3 scale)
{
	scaleTransformer(SelectType::eObject, pObject, scale);
}
inline void scaleTransformer(tke::Terrain *pTerrain, glm::vec3 scale)
{
	scaleTransformer(SelectType::eTerrain, pTerrain, scale);
}
inline void scaleTransformer(tke::Rigidbody *pRigidbody, glm::vec3 scale)
{
	scaleTransformer(SelectType::eRigidbody, pRigidbody, scale);
}
inline void scaleTransformer(tke::Shape *pShape, glm::vec3 scale)
{
	scaleTransformer(SelectType::eShape, pShape, scale);
}
inline void scaleTransformer(tke::Joint *pJoint, glm::vec3 scale)
{
	scaleTransformer(SelectType::eJoint, pJoint, scale);
}

void rotateTransformerAxis(SelectType selectType, tke::Transformer *pTrans, tke::Transformer::Axis which, float ang);

inline void rotateTransformerAxis(tke::Light *pLight, tke::Transformer::Axis which, float ang)
{
	rotateTransformerAxis(SelectType::eLight, pLight, which, ang);
}
inline void rotateTransformerAxis(tke::Object *pObject, tke::Transformer::Axis which, float ang)
{
	rotateTransformerAxis(SelectType::eObject, pObject, which, ang);
}
inline void rotateTransformerAxis(tke::Terrain *pTerrain, tke::Transformer::Axis which, float ang)
{
	rotateTransformerAxis(SelectType::eTerrain, pTerrain, which, ang);
}
inline void rotateTransformerAxis(tke::Rigidbody *pRigidbody, tke::Transformer::Axis which, float ang)
{
	rotateTransformerAxis(SelectType::eRigidbody, pRigidbody, which, ang);
}
inline void rotateTransformerAxis(tke::Shape *pShape, tke::Transformer::Axis which, float ang)
{
	rotateTransformerAxis(SelectType::eShape, pShape, which, ang);
}
inline void rotateTransformerAxis(tke::Joint *pJoint, tke::Transformer::Axis which, float ang)
{
	rotateTransformerAxis(SelectType::eJoint, pJoint, which, ang);
}

#endif