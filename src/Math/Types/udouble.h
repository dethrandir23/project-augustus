// src/Math/Types/udouble.h
#pragma once
#include <iostream>
#include <cmath>
#include <algorithm>
#include <nlohmann/json.hpp>

/**
 * @brief A safe 64-bit float (double) wrapper ensuring non-negative values.
 * Prevents NaN and Infinity propagation in macro-economic calculations.
 */
struct udouble {
    double value;

    // Constructor & Assignment
    udouble(double v = 0.0) : value(std::max(0.0, v)) {}

    operator double() const { return value; }

    udouble& operator=(double v) {
        value = std::max(0.0, v);
        return *this;
    }

    // Compound Assignment Operators
    udouble& operator+=(double v) {
        value = std::max(0.0, value + v);
        return *this;
    }

    udouble& operator-=(double v) {
        value = std::max(0.0, value - v);
        return *this;
    }

    udouble& operator*=(double v) {
        value = std::max(0.0, value * v);
        return *this;
    }

    udouble& operator/=(double v) {
        if (v != 0.0) {
            double res = value / v;
            value = (std::isnan(res) || std::isinf(res)) ? 0.0 : std::max(0.0, res);
        } else {
            value = 0.0;
        }
        return *this;
    }

    // Binary Operators (Simetrik ve Güvenli Tasarım)
    friend udouble operator+(udouble lhs, double rhs)  { return udouble(lhs.value + rhs); }
    friend udouble operator+(double lhs, udouble rhs)  { return udouble(lhs + rhs.value); }
    friend udouble operator+(udouble lhs, udouble rhs) { return udouble(lhs.value + rhs.value); }

    friend udouble operator-(udouble lhs, double rhs)  { return udouble(lhs.value - rhs); }
    friend udouble operator-(double lhs, udouble rhs)  { return udouble(lhs - rhs.value); }
    friend udouble operator-(udouble lhs, udouble rhs) { return udouble(lhs.value - rhs.value); }

    friend udouble operator*(udouble lhs, double rhs)  { return udouble(lhs.value * rhs); }
    friend udouble operator*(double lhs, udouble rhs)  { return udouble(lhs * rhs.value); }
    friend udouble operator*(udouble lhs, udouble rhs) { return udouble(lhs.value * rhs.value); }

    friend udouble operator/(udouble lhs, double rhs) {
        if (rhs == 0.0) return udouble(0.0);
        double res = lhs.value / rhs;
        return (std::isnan(res) || std::isinf(res)) ? udouble(0.0) : udouble(res);
    }
    friend udouble operator/(double lhs, udouble rhs) {
        if (rhs.value == 0.0) return udouble(0.0);
        double res = lhs / rhs.value;
        return (std::isnan(res) || std::isinf(res)) ? udouble(0.0) : udouble(res);
    }
    friend udouble operator/(udouble lhs, udouble rhs) {
        if (rhs.value == 0.0) return udouble(0.0);
        double res = lhs.value / rhs.value;
        return (std::isnan(res) || std::isinf(res)) ? udouble(0.0) : udouble(res);
    }

    // Comparison Operators
    bool operator==(const udouble& other) const { return value == other.value; }
    bool operator!=(const udouble& other) const { return value != other.value; }
    bool operator<(const udouble& other) const  { return value < other.value; }
    bool operator>(const udouble& other) const  { return value > other.value; }
    bool operator<=(const udouble& other) const { return value <= other.value; }
    bool operator>=(const udouble& other) const { return value >= other.value; }

    // Ostream
    friend std::ostream& operator<<(std::ostream& os, const udouble& d) {
        os << d.value;
        return os;
    }
};

// JSON Serialization
inline void to_json(nlohmann::json& j, const udouble& d) {
    j = d.value;
}

inline void from_json(const nlohmann::json& j, udouble& d) {
    d.value = std::max(0.0, j.get<double>());
}
