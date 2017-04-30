#ifndef __TKE_EDITOR_HISTORY__
#define __TKE_EDITOR_HISTORY__

#include "..\..\core\object.h"
#include "select.h"

namespace tke
{
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

		TransformHistory(SelectType selectType, int selectID, Transformer::Type transformType, glm::vec3 transformValue);
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
		Model *pModel;
		bool phyx;
		Object::MoveType moveType;
		Object::UpMethod upMethod;
		float baseForwardAng;
		float frontSpeed, backSpeed, leftSpeed, rightSpeed, upSpeed, downSpeed;
		glm::vec2 turnSpeed;

		ObjectCreationHistory(Object *pObject, CreationType _type);
		virtual void operate(Operate);
	};
}

#endif