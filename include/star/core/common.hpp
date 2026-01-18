#pragma once

// clang-format off

#include "platform.hpp"
#include "export.hpp"

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <chrono>
#include <ctime>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <memory>
#include <new>

#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>
#include <stack>
#include <list>
#include <forward_list>

#include <algorithm>
#include <functional>
#include <utility>
#include <type_traits>
#include <initializer_list>
#include <tuple>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <limits>
#include <random>
#include <numeric>
#include <regex>
#include <ranges>

#ifdef STAR_PLATFORM_APPLE
#include <TargetConditionals.h>
#endif

#ifdef STAR_PLATFORM_WINDOWS
#define NOMINMAX
#if __has_include(<sdkddkver.h>)
#include <sdkddkver.h>
#else
#include "sdkddkver.h"
#endif
#include <winsock2.h>
#include <windows.h>
#endif

// BGFX
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <bx/timer.h>

// SDL3
#include <SDL3/SDL.h>

// Logging
#include <spdlog/spdlog.h>

#include "types.hpp"
#include "version.hpp"
#include "assert.hpp"
#include "memory/optional_ref.hpp"

using namespace std::literals;
using namespace std::chrono_literals;

// clang-format on
