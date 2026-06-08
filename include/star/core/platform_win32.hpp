#pragma once

#include "platform.hpp"

#ifdef STAR_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#if __has_include(<sdkddkver.h>)
#  include <sdkddkver.h>
#endif

#include <windows.h>

#ifdef near
#  undef near
#endif
#ifdef far
#  undef far
#endif
#ifdef GetObject
#  undef GetObject
#endif
#ifdef DrawText
#  undef DrawText
#endif

#endif // STAR_PLATFORM_WINDOWS
