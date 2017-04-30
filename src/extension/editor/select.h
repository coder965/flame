#ifndef __TKE_EDITOR_SELECT__
#define __TKE_EDITOR_SELECT__

namespace tke
{
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
}

#endif