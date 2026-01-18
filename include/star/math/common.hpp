#pragma once
#include <algorithm>
#include <cmath>

#include "star/core/types.hpp"

namespace star::math {
    template<typename T>
    struct Constants {
        static constexpr T pi = T(3.14159265358979323846);
        static constexpr T two_pi = T(6.28318530717958647692);
        static constexpr T half_pi = T(1.57079632679489661923);
        static constexpr T quarter_pi = T(0.78539816339744830962);
        static constexpr T e = T(2.71828182845904523536);
        static constexpr T sqrt2 = T(1.41421356237309504880);
        static constexpr T sqrt3 = T(1.73205080756887729352);
        static constexpr T epsilon = T(0.00001);
        static constexpr T deg_to_rad = pi / T(180);
        static constexpr T rad_to_deg = T(180) / pi;
    };

    using Constantsf = Constants<f32>;
    using Constantsd = Constants<f64>;

    template<typename T>
    constexpr T radians(T degrees) {
        return degrees * Constants<T>::deg_to_rad;
    }

    template<typename T>
    constexpr T degrees(T radians) {
        return radians * Constants<T>::rad_to_deg;
    }

    template<typename T>
    constexpr T lerp(T a, T b, T t) {
        return a + t * (b - a);
    }

    template<typename T>
    constexpr T clamp(T value, T min, T max) {
        return std::max(min, std::min(max, value));
    }

    template<typename T>
    constexpr bool approximately(T a, T b, T epsilon = Constants<T>::epsilon) {
        return std::abs(a - b) <= epsilon;
    }

    template<typename T>
    constexpr T sign(T value) {
        return (T(0) < value) - (value < T(0));
    }

    template<typename T>
    constexpr T smoothstep(T edge0, T edge1, T x) {
        T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
        return t * t * (T(3) - T(2) * t);
    }

    template<typename T>
    constexpr T smootherstep(T edge0, T edge1, T x) {
        T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
        return t * t * t * (t * (t * T(6) - T(15)) + T(10));
    }
} // namespace star::math
