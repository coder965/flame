#ifndef __TK_SOUND__
#define __TK_SOUND__

#include <vector>
#include <al.h>  
#include <alc.h>

#include "math.h"

extern ALCdevice *alcDevice;
extern ALCcontext *alcContext;

extern std::vector<ALuint> soundBuffers;

namespace tke
{
	void setSoundListenerPos(const glm::vec3 &p);

	void setSoundListenerDir(const glm::vec3 &p);

	struct SoundObject
	{
		ALuint obj;

		SoundObject();
		~SoundObject();
		void setCoord(const glm::vec3 &p);
	};

	void initSound();
}

#endif