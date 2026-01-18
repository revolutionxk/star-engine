#pragma once

#ifdef _WIN32
#define STAR_PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define STAR_PLATFORM_APPLE
#if TARGET_OS_IPHONE
#define STAR_PLATFORM_IOS
#elif TARGET_OS_MAC
#define STAR_PLATFORM_MACOS
#endif
#elif defined(__linux__)
#define STAR_PLATFORM_LINUX
#elif defined(__ANDROID__)
#define STAR_PLATFORM_ANDROID
#endif
