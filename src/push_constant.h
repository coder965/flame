#pragma once

#include "utils.h"

namespace tke
{
	REFLECTABLE struct PushConstantRange
	{
		REFL_BANK;

		REFLv int offset = 0;
		REFLv int size = 0;
	};
}
