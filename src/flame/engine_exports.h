
#ifndef FLAME_ENGINE_EXPORTS_H
#define FLAME_ENGINE_EXPORTS_H

#ifdef SHARED_EXPORTS_BUILT_AS_STATIC
#  define FLAME_ENGINE_EXPORTS
#  define FLAME_ENGINE_NO_EXPORT
#else
#  ifndef FLAME_ENGINE_EXPORTS
#    ifdef flame_engine_EXPORTS
        /* We are building this library */
#      define FLAME_ENGINE_EXPORTS 
#    else
        /* We are using this library */
#      define FLAME_ENGINE_EXPORTS 
#    endif
#  endif

#  ifndef FLAME_ENGINE_NO_EXPORT
#    define FLAME_ENGINE_NO_EXPORT 
#  endif
#endif

#ifndef FLAME_ENGINE_DEPRECATED
#  define FLAME_ENGINE_DEPRECATED __declspec(deprecated)
#endif

#ifndef FLAME_ENGINE_DEPRECATED_EXPORT
#  define FLAME_ENGINE_DEPRECATED_EXPORT FLAME_ENGINE_EXPORTS FLAME_ENGINE_DEPRECATED
#endif

#ifndef FLAME_ENGINE_DEPRECATED_NO_EXPORT
#  define FLAME_ENGINE_DEPRECATED_NO_EXPORT FLAME_ENGINE_NO_EXPORT FLAME_ENGINE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FLAME_ENGINE_NO_DEPRECATED
#    define FLAME_ENGINE_NO_DEPRECATED
#  endif
#endif

#endif
