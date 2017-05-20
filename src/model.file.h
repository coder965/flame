#ifndef __TKE_MODEL_ARCHIVE__
#define __TKE_MODEL_ARCHIVE__

#include <fstream>

#include "model.h"

namespace tke
{
	namespace OBJ
	{
		void load(Model *m, std::ifstream &file);
	}

	namespace PMD
	{
		void load(Model *m, std::ifstream &file);
	}

	namespace VMD
	{

		void load(AnimationTemplate *a, std::ifstream &file);
	}

	namespace TKM
	{
		void load(Model *m, std::ifstream &file);
		void save(Model *, const std::string &filename, bool copyTexture);
	}

	namespace TKA
	{
		void load(AnimationTemplate *a, std::ifstream &file);
		void save(AnimationTemplate *a, const std::string &filename);
	}

	Model *createModel(const std::string &filename);

	AnimationTemplate *createAnimation(const std::string &filename);
}

#endif