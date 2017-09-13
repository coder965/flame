#include "define.h"

namespace tke
{
	int lastTime = 0;
	int nowTime = 0;
	int timeDisp;

	std::string enginePath;

	int resCx;
	int resCy;

	float screenAspect;

	glm::mat4 matOrtho;
	glm::mat4 matOrthoInv;
	glm::mat4 matPerspective;
	glm::mat4 matPerspectiveInv;
}
