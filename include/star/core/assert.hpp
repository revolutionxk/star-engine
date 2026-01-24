#pragma once

#if defined(_DEBUG) || defined(DEBUG)
    #define STAR_DEBUG_BUILD
#endif

namespace star {
    class Logger;
}

#ifdef STAR_DEBUG_BUILD
    #define STAR_CORE_ASSERT(condition, ...) \
        do { \
            if (!(condition)) { \
                ::star::Logger::get_core_logger()->critical("Assertion Failed: {}", fmt::format(__VA_ARGS__)); \
                ::star::Logger::get_core_logger()->critical("  Location: {}:{}", __FILE__, __LINE__); \
                __debugbreak(); \
            } \
        } while(0)

    #define STAR_ASSERT(condition, ...) \
        do { \
            if (!(condition)) { \
                ::star::Logger::get_client_logger()->critical("Assertion Failed: {}", fmt::format(__VA_ARGS__)); \
                ::star::Logger::get_client_logger()->critical("  Location: {}:{}", __FILE__, __LINE__); \
                __debugbreak(); \
            } \
        } while(0)

    #define STAR_VERIFY(condition, ...) STAR_CORE_ASSERT(condition, __VA_ARGS__)
#else
    #define STAR_CORE_ASSERT(condition, ...) ((void)0)
    #define STAR_ASSERT(condition, ...) ((void)0)
    #define STAR_VERIFY(condition, ...) (condition)
#endif

#define STAR_STATIC_ASSERT(condition, msg) static_assert(condition, msg)
