#ifndef FLAME_EXPORTS_H
#define FLAME_EXPORTS_H

#ifdef TK_EXPORTS
	#define FLAME_EXPORTS __declspec(dllexport)
#else
	#define FLAME_EXPORTS __declspec(dllimport)
#endif

#endif
