#pragma once

#ifdef _FLAME_SHADER_EXPORTS
#define FLAME_SHADER_EXPORTS __declspec(dllexport)
#else
#define FLAME_SHADER_EXPORTS __declspec(dllimport)
#endif

namespace flame
{
}

