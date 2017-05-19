#include "history.h"
#include "..\src\core\core.h"
#include "..\src\core\scene.h"

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

TransformHistory::TransformHistory(SelectType selectType, int selectID, tke::Transformer::Type transformType, glm::vec3 transformValue)
	: m_selectType(selectType), m_selectID(selectID), m_transformType(transformType), m_transformValue(transformValue)
{
	type = History::Type::eTransform;
}

void TransformHistory::operate(Operate op)
{
	auto transformer = (tke::Transformer*)getSelectPtr(m_selectType, m_selectID);
	if (!transformer) return;
	auto sign = 1.f;
	if (op == Operate::eUndo) sign = -1.f;
	switch (m_transformType)
	{
	case tke::Transformer::Type::eMove:
		transformer->addCoord(sign * m_transformValue);
		break;
	case tke::Transformer::Type::eEulerSet:
		transformer->addEuler(sign * m_transformValue);
		break;
	case tke::Transformer::Type::eScale:
		transformer->addScale(sign * m_transformValue);
		break;
	case tke::Transformer::Type::eAsixRotate:
		transformer->axisRotate((tke::Transformer::Axis)(int)m_transformValue.x, sign * m_transformValue.y);
		break;
	}
}

ObjectCreationHistory::ObjectCreationHistory(tke::Object *pObject, CreationType _type)
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
		auto pObject = new tke::Object;
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
		tke::scene->addObject(pObject, id);
	}
	else
	{
		auto pObject = (tke::Object*)getSelectPtr(SelectType::eObject, id);
		if (pObject) tke::scene->deleteObject(pObject);
	}
}

std::vector<History*> histories;
int currentHistory;

void addHistory(History *history)
{
	for (int i = currentHistory; i < histories.size(); i++)
		delete histories[i];
	histories.resize(currentHistory);
	histories.push_back(history);
	currentHistory++;
}

void undo()
{
	if (currentHistory > 0)
	{
		auto pHistory = histories[(currentHistory - 1)];
		pHistory->operate(History::Operate::eUndo);
		currentHistory--;
	}
}

void redo()
{
	if (currentHistory < histories.size())
	{
		auto pHistory = histories[currentHistory];
		pHistory->operate(History::Operate::eRedo);
		currentHistory++;
	}
}

void select()
{
	if (SelectType::eNull == selectType)
		return;

	addHistory(new SelectionHistory);
	selectType = SelectType::eNull;

	tke::needRedraw = true;
}

void select(SelectType type, void *ptr)
{
	if (type == selectType && ptr == selecting)
		return;

	if (!ptr)
	{
		select();
		return;
	}
	addHistory(new SelectionHistory);
	selectType = type;
	selecting = ptr;

	tke::needRedraw = true;
}

void beginRecordTransformHistory()
{
	leftTransformerHistory = true;
	leftTransformerHistoryFirstHistory = true;
}

void endRecordTransformHistory()
{
	if (leftTransformerHistory)
	{
		addHistory(new TransformHistory(leftTransformerHistorySelectType, leftTransformerHistorySelectID, leftTransformerHistoryType, leftTransformerHistoryValue - leftTransformerHistoryOriginalValue));
		leftTransformerHistory = false;
	}
}

void moveTransformer(SelectType selectType, tke::Transformer *pTrans, glm::vec3 coord)
{
	auto originalCoord = pTrans->getCoord();
	if (!leftTransformerHistory)
	{
		addHistory(new TransformHistory(selectType, pTrans->m_id, tke::Transformer::Type::eMove, coord - originalCoord));
	}
	else
	{
		if (leftTransformerHistoryFirstHistory)
		{
			leftTransformerHistoryOriginalValue = originalCoord;
			leftTransformerHistoryFirstHistory = false;
		}
		leftTransformerHistorySelectType = selectType;
		leftTransformerHistorySelectID = pTrans->m_id;
		leftTransformerHistoryType = tke::Transformer::Type::eMove;
		leftTransformerHistoryValue = coord;
	}
	pTrans->setCoord(coord);
}

void setTransformerEuler(SelectType selectType, tke::Transformer *pTrans, glm::vec3 euler)
{
	auto originalEuler = pTrans->getEuler();
	if (!leftTransformerHistory)
	{
		addHistory(new TransformHistory(selectType, pTrans->m_id, tke::Transformer::Type::eEulerSet, euler - originalEuler));
	}
	else
	{
		if (leftTransformerHistoryFirstHistory)
		{
			leftTransformerHistoryOriginalValue = originalEuler;
			leftTransformerHistoryFirstHistory = false;
		}
		leftTransformerHistorySelectType = selectType;
		leftTransformerHistorySelectID = pTrans->m_id;
		leftTransformerHistoryType = tke::Transformer::Type::eEulerSet;
		leftTransformerHistoryValue = euler;
	}
	pTrans->setEuler(euler);
}

void scaleTransformer(SelectType selectType, tke::Transformer *pTrans, glm::vec3 scale)
{
	auto originalScale = pTrans->getScale();
	if (!leftTransformerHistory)
	{
		addHistory(new TransformHistory(selectType, pTrans->m_id, tke::Transformer::Type::eScale, scale - originalScale));
	}
	else
	{
		if (leftTransformerHistoryFirstHistory)
		{
			leftTransformerHistoryOriginalValue = originalScale;
			leftTransformerHistoryFirstHistory = false;
		}
		leftTransformerHistorySelectType = selectType;
		leftTransformerHistorySelectID = pTrans->m_id;
		leftTransformerHistoryType = tke::Transformer::Type::eScale;
		leftTransformerHistoryValue = scale;
	}
	pTrans->setScale(scale);
}

void rotateTransformerAxis(SelectType selectType, tke::Transformer *pTrans, tke::Transformer::Axis which, float ang)
{
	glm::vec3 v = glm::vec3((float)which, ang, 0.f);
	if (!leftTransformerHistory)
	{
		addHistory(new TransformHistory(selectType, pTrans->m_id, tke::Transformer::Type::eAsixRotate, v));
	}
	else
	{
		if (leftTransformerHistoryFirstHistory)
		{
			leftTransformerHistoryOriginalValue = v;
			leftTransformerHistoryFirstHistory = false;
		}
		leftTransformerHistorySelectType = selectType;
		leftTransformerHistorySelectID = pTrans->m_id;
		leftTransformerHistoryType = tke::Transformer::Type::eAsixRotate;
		leftTransformerHistoryValue = v;
	}
	pTrans->axisRotate(which, ang);
}
