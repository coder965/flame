
#ifndef FLAME_COMMON_EXPORTS_H
#define FLAME_COMMON_EXPORTS_H

#ifdef SHARED_EXPORTS_BUILT_AS_STATIC
#  define FLAME_COMMON_EXPORTS
#  define FLAME_COMMON_NO_EXPORT
#else
#  ifndef FLAME_COMMON_EXPORTS
#    ifdef flame_common_EXPORTS
        /* We are building this library */
#      define FLAME_COMMON_EXPORTS 
#    else
        /* We are using this library */
#      define FLAME_COMMON_EXPORTS 
#    endif
#  endif

#  ifndef FLAME_COMMON_NO_EXPORT
#    define FLAME_COMMON_NO_EXPORT 
#  endif
#endif

#ifndef FLAME_COMMON_DEPRECATED
#  define FLAME_COMMON_DEPRECATED __declspec(deprecated)
#endif

#ifndef FLAME_COMMON_DEPRECATED_EXPORT
#  define FLAME_COMMON_DEPRECATED_EXPORT FLAME_COMMON_EXPORTS FLAME_COMMON_DEPRECATED
#endif

#ifndef FLAME_COMMON_DEPRECATED_NO_EXPORT
#  define FLAME_COMMON_DEPRECATED_NO_EXPORT FLAME_COMMON_NO_EXPORT FLAME_COMMON_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FLAME_COMMON_NO_DEPRECATED
#    define FLAME_COMMON_NO_DEPRECATED
#  endif
#endif

#endif
