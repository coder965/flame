#pragma once

#include <string>

#if defined(_WIN64)
typedef __int64 TK_LONG_PTR;
#else
typedef _W64 long TK_LONG_PTR;
#endif

#define TK_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define TK_DERIVE_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)
#define TK_LOW(I) ((I) & 0xffff)
#define TK_HIGH(I) ((I) >> 16)
#define TK_MAKEINT(H, L) ((L) | ((H) << 16))
#define TK_INIT_BEGINE(n) struct n##_init{n##_init(){
#define TK_INIT_END }};

template<size_t s> struct Sizer {};

typedef void(*PF_EVENT0)();
typedef void(*PF_EVENT1)(int);
typedef void(*PF_EVENT2)(int, int);

namespace tke
{
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;

	extern bool only_2d;

	enum Err
	{
		NoErr,
		ErrInvalidEnum,
		ErrInvalidValue,
		ErrInvalidOperation,
		ErrOutOfMemory,
		ErrContextLost,
		ErrResourceLost
	};

	std::string get_error_string(Err errNum);

	extern float near_plane;
	extern float far_plane;
	extern float fovy;

	extern int nowTime;

	extern std::string engine_path;

	extern unsigned long long total_frame_count;
	extern uint32_t FPS;

	enum DescriptorSetBindings
	{
		MainDescriptorSetBinding,
		MaterialDescriptorSetBinding,
		BoneSetDescriptorBinding
	};

	enum MainDescriptorSetBindings
	{
		ConstantBufferDescriptorBinding,
		MatrixBufferDescriptorBinding,
		ObjectMatrixBufferDescriptorBinding,
		TerrainBufferDescriptorBinding,
		TerrainBlendImageDescriptorBinding,
		WaterBufferDescriptorBinding,
		DepthImageDescriptorBinding,
		AlbedoAlphaImageDescriptorBinding,
		NormalHeightImageDescriptorBinding,
		SpecRoughnessDescriptorImageBinding,
		LightBufferDescriptorBinding,
		EnvrImageDescriptorBinding,
		AmbientBufferDescriptorBinding,
		AoImageDescriptorBinding,
		ShadowBufferDescriptorBinding,
		ShadowImageDescriptorBinding
	};

	enum MaterialDescriptorSetBindings
	{
		MaterialBufferDescriptorBinding,
		MaterialImagesDescriptorBinding
	};

	enum Op
	{
		OpNeedRemove,
		OpKeep,
		OpNeedUpdate
	};
}
