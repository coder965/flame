#include "terrain.h"

namespace tke
{
	void Terrain::getRefrence()
	{
		refrenceCount++;
	}

	void Terrain::release()
	{
		refrenceCount--;
		if (refrenceCount == 0)
			delete this;
	}
}

