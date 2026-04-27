#pragma once
#include <catch2/catch_approx.hpp>

#include "star/math/common.hpp"

namespace test {
    inline Catch::Approx approx(const float value, const float margin = 1e-4f) {
        return Catch::Approx(value).margin(margin);
    }

    inline Catch::Approx approxd(const double value, const double margin = 1e-6) {
        return Catch::Approx(value).margin(margin);
    }
} // namespace test
