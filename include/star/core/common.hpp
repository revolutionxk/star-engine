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
#include <typeindex>
#include <stdexcept>
#include <memory>
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
#undef far
#undef near
#if __has_include(<sdkddkver.h>)
#include <sdkddkver.h>
#else
#include "sdkddkver.h"
#endif
#include <winsock2.h>
#include <windows.h>
#endif

// Logging
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "types.hpp"
#include "version.hpp"
#include "logger.hpp"
#include "assert.hpp"
#include "memory/optional_ref.hpp"
#include "star/math/math.hpp"

using namespace std::literals;
using namespace std::chrono_literals;

// clang-format on
