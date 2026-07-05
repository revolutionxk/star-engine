#pragma once

#if defined(STAR_STATIC_DEFINE) || !defined(STAR_SHARED_LIB)
    #define STAR_EXPORT
    #define STAR_NO_EXPORT
#else
    #ifndef STAR_EXPORT
        #ifdef _WIN32
            #ifdef STAR_EXPORTS
                #define STAR_EXPORT __declspec(dllexport)
            #else
                #define STAR_EXPORT __declspec(dllimport)
            #endif
        #else
            #ifdef STAR_EXPORTS
                #define STAR_EXPORT __attribute__((visibility("default")))
            #else
                #define STAR_EXPORT
            #endif
        #endif
    #endif

    #ifndef STAR_NO_EXPORT
        #ifdef _WIN32
            #define STAR_NO_EXPORT
        #else
            #define STAR_NO_EXPORT __attribute__((visibility("hidden")))
        #endif
    #endif
#endif

#ifndef STAR_DEPRECATED
    #ifdef _MSC_VER
        #define STAR_DEPRECATED __declspec(deprecated)
    #else
        #define STAR_DEPRECATED __attribute__((deprecated))
    #endif
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
