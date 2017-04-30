#include "editor.h"
#include "..\..\core\scene.h"

namespace tke
{
	SelectionHistory::SelectionHistory()
	{
		m_selectType = selectType;
		if (m_selectType != SelectType::eNull) m_id = selectTransformer()->m_id;
		type = Type::eSelection;
	}

	void SelectionHistory::operate(Operate op)
	{
		auto typePrev = selectType;
		int idPrev = 0;
		if (typePrev != SelectType::eNull) idPrev = selectTransformer()->m_id;
		selectType = m_selectType;
		selecting = getSelectPtr(m_selectType, m_id);
		if (!selecting) selectType = SelectType::eNull;
		m_selectType = typePrev;
		m_id = idPrev;
	}

	TransformHistory::TransformHistory(SelectType selectType, int selectID, Transformer::Type transformType, glm::vec3 transformValue)
		: m_selectType(selectType), m_selectID(selectID), m_transformType(transformType), m_transformValue(transformValue)
	{
		type = History::Type::eTransform;
	}

	void TransformHistory::operate(Operate op)
	{
		auto transformer = (Transformer*)getSelectPtr(m_selectType, m_selectID);
		if (!transformer) return;
		auto sign = 1.f;
		if (op == Operate::eUndo) sign = -1.f;
		switch (m_transformType)
		{
		case Transformer::Type::eMove:
			transformer->addCoord(sign * m_transformValue);
			break;
		case Transformer::Type::eEulerSet:
			transformer->addEuler(sign * m_transformValue);
			break;
		case Transformer::Type::eScale:
			transformer->addScale(sign * m_transformValue);
			break;
		case Transformer::Type::eAsixRotate:
			transformer->axisRotate((Transformer::Axis)(int)m_transformValue.x, sign * m_transformValue.y);
			break;
		}
	}

	ObjectCreationHistory::ObjectCreationHistory(Object *pObject, CreationType _type)
		: creationType(_type)
	{
		id = pObject->m_id;
		coord = pObject->getCoord();
		euler = pObject->getEuler();
		scale = pObject->getScale();
		pModel = pObject->pModel;
		phyx = pObject->phyx;
		moveType = pObject->moveType;
		upMethod = pObject->upMethod;
		baseForwardAng = pObject->baseForwardAng;
		frontSpeed = pObject->frontSpeed;
		backSpeed = pObject->backSpeed;
		leftSpeed = pObject->leftSpeed;
		rightSpeed = pObject->rightSpeed;
		upSpeed = pObject->upSpeed;
		downSpeed = pObject->downSpeed;
		turnSpeed = pObject->turnSpeed;

		type = Type::eObjectCreation;
	}

	void ObjectCreationHistory::operate(Operate op)
	{
		if ((op == Operate::eUndo && creationType == CreationType::eDelete) ||
			(op == Operate::eRedo && creationType == CreationType::eCreate))
		{
			auto pObject = new Object;
			pObject->pModel = pModel;
			pObject->setCoord(coord);
			pObject->setEuler(euler);
			pObject->setScale(scale);
			pObject->phyx = phyx;
			pObject->moveType = moveType;
			pObject->upMethod = upMethod;
			pObject->baseForwardAng = baseForwardAng;
			pObject->frontSpeed = frontSpeed;
			pObject->backSpeed = backSpeed;
			pObject->leftSpeed = leftSpeed;
			pObject->rightSpeed = rightSpeed;
			pObject->upSpeed = upSpeed;
			pObject->downSpeed = downSpeed;
			pObject->turnSpeed = turnSpeed;
			scene->addObject(pObject, id);
		}
		else
		{
			auto pObject = (Object*)getSelectPtr(SelectType::eObject, id);
			if (pObject) scene->deleteObject(pObject);
		}
	}
}
