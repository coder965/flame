#ifndef __TKE_IMAGE_FILE__
#define __TKE_IMAGE_FILE__

#include "../core/render.h"

namespace tke
{
	Image *createImage(const std::string &filename, bool sRGB, bool saveData = false);
}

#endif