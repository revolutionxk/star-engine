#pragma once

#ifdef STAR_STATIC_DEFINE
    #define STAR_EXPORT
    #define STAR_NO_EXPORT
#else
    #ifndef STAR_EXPORT
        #ifdef STAR_EXPORTS
            #define STAR_EXPORT
        #else
            #define STAR_EXPORT
        #endif
    #endif

    #ifndef STAR_NO_EXPORT
        #define STAR_NO_EXPORT
    #endif
#endif

#ifndef STAR_DEPRECATED
    #define STAR_DEPRECATED __declspec(deprecated)
#endif

#ifndef STAR_DEPRECATED_EXPORT
    #define STAR_DEPRECATED_EXPORT STAR_EXPORT STAR_DEPRECATED
#endif

#ifndef STAR_DEPRECATED_NO_EXPORT
    #define STAR_DEPRECATED_NO_EXPORT STAR_NO_EXPORT STAR_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
    #ifndef STAR_NO_DEPRECATED
        #define STAR_NO_DEPRECATED
    #endif
#endif
