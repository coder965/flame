#include <flame/engine/sound/sound.h>

//ALCdevice *alcDevice = nullptr;
//ALCcontext *alcContext = nullptr;
//
//std::vector<ALuint> soundBuffers;
//
//namespace flame
//{
//	void setSoundListenerPos(const glm::vec3 &p)
//	{
//		alListenerfv(AL_POSITION, &p[0]);
//	}
//
//	void setSoundListenerDir(const glm::vec3 &p)
//	{
//		float v[] = {
//			p.x, p.y, p.z,
//			0.f, 1.f, 0.f
//		};
//		alListenerfv(AL_ORIENTATION, v);
//	}
//
//	SoundObject::SoundObject()
//	{
//		alGenSources(1, &obj);
//		alSourcei(obj, AL_BUFFER, soundBuffers[0]);
//		alSourcef(obj, AL_GAIN, 0.5f);
//		alSourcei(obj, AL_LOOPING, true);
//		alSourcePlay(obj);
//	}
//
//	SoundObject::~SoundObject()
//	{
//		alSourceStop(obj);
//	}
//
//	void SoundObject::setCoord(const glm::vec3 &p)
//	{
//		alSourcefv(obj, AL_POSITION, &p[0]);
//	}
//
	void init_sound()
	{
		//alcDevice = alcOpenDevice(nullptr);
		//alcContext = alcCreateContext(alcDevice, nullptr);
		//alcMakeContextCurrent(alcContext);

		//unsigned char buf[512];
		//for (int i = 0; i < 512; i++)
		//	buf[i] = (glm::sin((i / 128.f) * M_PI) + 1.f) * 0.5f * 255;

		//ALuint buffer;
		//alGenBuffers(1, &buffer);
		//alBufferData(buffer, AL_FORMAT_STEREO8, buf, 512, 16000);
		//soundBuffers.push_back(buffer);
	}
//}
