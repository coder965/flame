#ifndef __TKE_OBJECT__
#define __TKE_OBJECT__

#include "transformer.h"
#include "controler.h"
#include "model.h"

namespace tke
{
	struct Model;
	struct Object : Transformer, Controller
	{
		enum class MoveType : int
		{
			eNormal,
			eByAnimationLockY
		};

		enum class UpMethod : int
		{
			eBan,
			eJet,
			eJump
		};

		int refrenceCount = 0;
		bool dying = false;

		Model *pModel = nullptr;
		bool phyx = false;
		MoveType moveType = MoveType::eNormal;
		UpMethod upMethod = UpMethod::eJump;

		AnimationSolver *animationSolver;

		float floatingTime = 0.f;
		RigidData *rigidDatas = nullptr;

		int sceneIndex = -1;

		Object();
		~Object();
		void getRefrence();
		void release();
	};
}

#endif