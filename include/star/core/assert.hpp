#pragma once

#include "logger.hpp"

#if defined(_DEBUG) || defined(DEBUG)
    #define STAR_DEBUG_BUILD
#endif

#if defined(_MSC_VER)
    #define STAR_DEBUGBREAK() __debugbreak()
#elif defined(__clang__)
    #define STAR_DEBUGBREAK() __builtin_debugtrap()
#elif defined(__GNUC__)
    #define STAR_DEBUGBREAK() __builtin_trap()
#else
    #define STAR_DEBUGBREAK() std::abort()
#endif

#ifdef STAR_DEBUG_BUILD
    #define STAR_CORE_ASSERT(condition, ...) \
        do { \
            if (!(condition)) { \
                ::star::Logger::get_core_logger()->critical("Assertion Failed: {}", fmt::format(__VA_ARGS__)); \
                ::star::Logger::get_core_logger()->critical("  Location: {}:{}", __FILE__, __LINE__); \
                STAR_DEBUGBREAK(); \
            } \
        } while (0)

    #define STAR_ASSERT(condition, ...) \
        do { \
            if (!(condition)) { \
                ::star::Logger::get_client_logger()->critical("Assertion Failed: {}", fmt::format(__VA_ARGS__)); \
                ::star::Logger::get_client_logger()->critical("  Location: {}:{}", __FILE__, __LINE__); \
                STAR_DEBUGBREAK(); \
            } \
        } while (0)

    #define STAR_VERIFY(condition, ...) STAR_CORE_ASSERT(condition, __VA_ARGS__)
#else
    #define STAR_CORE_ASSERT(condition, ...) ((void)0)
    #define STAR_ASSERT(condition, ...) ((void)0)
    #define STAR_VERIFY(condition, ...) ((void)(condition))
#endif

#define STAR_STATIC_ASSERT(condition, msg) static_assert(condition, msg)
