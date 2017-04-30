#include <vector>

#include "editor.h"
#include "../image.file.h"
#include "../../core/scene.h"

namespace tke
{
	SelectType selectType = SelectType::eNull;
	void *selecting = nullptr;
	std::vector<History*> histories;
	int currentHistory = 0;
	Tool *currentTool = nullptr;
	TransformTool transformTool;

	void addHistory(History *his)
	{
		for (int i = currentHistory; i < histories.size(); i++)
			delete histories[i];
		histories.resize(currentHistory);
		histories.push_back(his);
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

	void setTool(Tool *pTool)
	{
		if (pTool == currentTool) return;
		if (currentTool) currentTool->release();
		currentTool = pTool;
		if (pTool) pTool->attach();
	}

	void select()
	{
		if (SelectType::eNull == selectType)
			return;

		addHistory(new SelectionHistory);
		selectType = SelectType::eNull;

		postRedrawRequest();
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

		postRedrawRequest();
	}

	Transformer *selectTransformer()
	{
		if (selectType == SelectType::eNull) return nullptr;
		return (Transformer*)selecting;
	}

	void *getSelectPtr(SelectType type, int id)
	{
		switch (type)
		{
		case SelectType::eRigidbody:
			return scene->getRigidbody(id);
		case SelectType::eShape:
			return scene->getShape(id);
		case SelectType::eJoint:
			return scene->getJoint(id);
		case SelectType::eLight:
			return scene->getLight(id);
		case SelectType::eObject:
			return scene->getObject(id);
		case SelectType::eTerrain:
			return scene->getTerrain(id);
		}
		return nullptr;
	}

	static auto leftTransformerHistory = false;
	static auto leftTransformerHistoryFirstHistory = false;
	static SelectType leftTransformerHistorySelectType;
	static int leftTransformerHistorySelectID;
	static Transformer::Type leftTransformerHistoryType;
	static glm::vec3 leftTransformerHistoryOriginalValue;
	static glm::vec3 leftTransformerHistoryValue;

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

	void moveTransformer(SelectType selectType, Transformer *pTrans, glm::vec3 coord)
	{
		auto originalCoord = pTrans->getCoord();
		if (!leftTransformerHistory)
		{
			addHistory(new TransformHistory(selectType, pTrans->m_id, Transformer::Type::eMove, coord - originalCoord));
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
			leftTransformerHistoryType = Transformer::Type::eMove;
			leftTransformerHistoryValue = coord;
		}
		pTrans->setCoord(coord);
	}

	void setTransformerEuler(SelectType selectType, Transformer *pTrans, glm::vec3 euler)
	{
		auto originalEuler = pTrans->getEuler();
		if (!leftTransformerHistory)
		{
			addHistory(new TransformHistory(selectType, pTrans->m_id, Transformer::Type::eEulerSet, euler - originalEuler));
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
			leftTransformerHistoryType = Transformer::Type::eEulerSet;
			leftTransformerHistoryValue = euler;
		}
		pTrans->setEuler(euler);
	}

	void scaleTransformer(SelectType selectType, Transformer *pTrans, glm::vec3 scale)
	{
		auto originalScale = pTrans->getScale();
		if (!leftTransformerHistory)
		{
			addHistory(new TransformHistory(selectType, pTrans->m_id, Transformer::Type::eScale, scale - originalScale));
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
			leftTransformerHistoryType = Transformer::Type::eScale;
			leftTransformerHistoryValue = scale;
		}
		pTrans->setScale(scale);
	}

	void rotateTransformerAxis(SelectType selectType, Transformer *pTrans, Transformer::Axis which, float ang)
	{
		glm::vec3 v = glm::vec3((float)which, ang, 0.f);
		if (!leftTransformerHistory)
		{
			addHistory(new TransformHistory(selectType, pTrans->m_id, Transformer::Type::eAsixRotate, v));
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
			leftTransformerHistoryType = Transformer::Type::eAsixRotate;
			leftTransformerHistoryValue = v;
		}
		pTrans->axisRotate(which, ang);
	}
}
