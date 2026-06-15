// src/Math/Types/real.h
#pragma once
#include <iostream>
#include <cmath>
#include <nlohmann/json.hpp>

/**
 * @brief A safe 64-bit float (double) wrapper.
 * Allows negative numbers but prevents NaN and Infinity propagation.
 */
struct real {
    double value;

    real(double v = 0.0) {
        value = (std::isnan(v) || std::isinf(v)) ? 0.0 : v;
    }

    operator double() const { return value; }

    real& operator=(double v) {
        value = (std::isnan(v) || std::isinf(v)) ? 0.0 : v;
        return *this;
    }

    real& operator+=(double v) {
        double res = value + v;
        value = (std::isnan(res) || std::isinf(res)) ? 0.0 : res;
        return *this;
    }

    real& operator-=(double v) {
        double res = value - v;
        value = (std::isnan(res) || std::isinf(res)) ? 0.0 : res;
        return *this;
    }

    real& operator*=(double v) {
        double res = value * v;
        value = (std::isnan(res) || std::isinf(res)) ? 0.0 : res;
        return *this;
    }

    real& operator/=(double v) {
        if (v != 0.0) {
            double res = value / v;
            value = (std::isnan(res) || std::isinf(res)) ? 0.0 : res;
        } else {
            value = 0.0;
        }
        return *this;
    }

    friend real operator+(real lhs, double rhs) { return real(lhs.value + rhs); }
    friend real operator+(double lhs, real rhs) { return real(lhs + rhs.value); }
    friend real operator+(real lhs, real rhs)   { return real(lhs.value + rhs.value); }

    friend real operator-(real lhs, double rhs) { return real(lhs.value - rhs); }
    friend real operator-(double lhs, real rhs) { return real(lhs - rhs.value); }
    friend real operator-(real lhs, real rhs)   { return real(lhs.value - rhs.value); }

    friend real operator*(real lhs, double rhs) { return real(lhs.value * rhs); }
    friend real operator*(double lhs, real rhs) { return real(lhs * rhs.value); }
    friend real operator*(real lhs, real rhs)   { return real(lhs.value * rhs.value); }

    friend real operator/(real lhs, double rhs) {
        if (rhs == 0.0) return real(0.0);
        return real(lhs.value / rhs);
    }
    friend real operator/(double lhs, real rhs) {
        if (rhs.value == 0.0) return real(0.0);
        return real(lhs / rhs.value);
    }
    friend real operator/(real lhs, real rhs) {
        if (rhs.value == 0.0) return real(0.0);
        return real(lhs.value / rhs.value);
    }

    bool operator==(const real& other) const { return value == other.value; }
    bool operator!=(const real& other) const { return value != other.value; }
    bool operator<(const real& other) const  { return value < other.value; }
    bool operator>(const real& other) const  { return value > other.value; }
    bool operator<=(const real& other) const { return value <= other.value; }
    bool operator>=(const real& other) const { return value >= other.value; }

    friend std::ostream& operator<<(std::ostream& os, const real& r) {
        os << r.value;
        return os;
    }
};

inline void to_json(nlohmann::json& j, const real& r) { j = r.value; }
inline void from_json(const nlohmann::json& j, real& r) { r.value = j.get<double>(); }
