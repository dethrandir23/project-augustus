// ufloat.h
#pragma once
#include <iostream>
#include <cmath>
#include <algorithm>
#include <nlohmann/json.hpp>

/**
 * @brief A wrapper for float to ensure it never drops below zero.
 * Useful for economic variables like quantity, price, and population.
 */
struct ufloat {
    float value;

    // Constructor & Assignment
    ufloat(float v = 0.0f) : value(std::max(0.0f, v)) {}

    operator float() const { return value; }

    ufloat& operator=(float v) {
        value = std::max(0.0f, v);
        return *this;
    }

    // Compound Assignment Operators
    ufloat& operator+=(float v) {
        value = std::max(0.0f, value + v);
        return *this;
    }

    ufloat& operator-=(float v) {
        value = std::max(0.0f, value - v);
        return *this;
    }

    ufloat& operator*=(float v) {
        value = std::max(0.0f, value * v);
        return *this;
    }

    ufloat& operator/=(float v) {
        if (v != 0.0f) {
            float res = value / v;
            value = (std::isnan(res) || std::isinf(res)) ? 0.0f : std::max(0.0f, res);
        } else {
            value = 0.0f;
        }
        return *this;
    }

    // Binary Operators (Simetrik ve Güvenli Tasarım)
    friend ufloat operator+(ufloat lhs, float rhs)  { return ufloat(lhs.value + rhs); }
    friend ufloat operator+(float lhs, ufloat rhs)  { return ufloat(lhs + rhs.value); }
    friend ufloat operator+(ufloat lhs, ufloat rhs) { return ufloat(lhs.value + rhs.value); }

    friend ufloat operator-(ufloat lhs, float rhs)  { return ufloat(lhs.value - rhs); }
    friend ufloat operator-(float lhs, ufloat rhs)  { return ufloat(lhs - rhs.value); }
    friend ufloat operator-(ufloat lhs, ufloat rhs) { return ufloat(lhs.value - rhs.value); }

    friend ufloat operator*(ufloat lhs, float rhs)  { return ufloat(lhs.value * rhs); }
    friend ufloat operator*(float lhs, ufloat rhs)  { return ufloat(lhs * rhs.value); }
    friend ufloat operator*(ufloat lhs, ufloat rhs) { return ufloat(lhs.value * rhs.value); }

    friend ufloat operator/(ufloat lhs, float rhs) {
        if (rhs == 0.0f) return ufloat(0.0f);
        float res = lhs.value / rhs;
        return (std::isnan(res) || std::isinf(res)) ? ufloat(0.0f) : ufloat(res);
    }
    friend ufloat operator/(float lhs, ufloat rhs) {
        if (rhs.value == 0.0f) return ufloat(0.0f);
        float res = lhs / rhs.value;
        return (std::isnan(res) || std::isinf(res)) ? ufloat(0.0f) : ufloat(res);
    }
    friend ufloat operator/(ufloat lhs, ufloat rhs) {
        if (rhs.value == 0.0f) return ufloat(0.0f);
        float res = lhs.value / rhs.value;
        return (std::isnan(res) || std::isinf(res)) ? ufloat(0.0f) : ufloat(res);
    }

    // Comparison Operators
    bool operator==(const ufloat& other) const { return value == other.value; }
    bool operator!=(const ufloat& other) const { return value != other.value; }
    bool operator<(const ufloat& other) const { return value < other.value; }
    bool operator>(const ufloat& other) const { return value > other.value; }
    bool operator<=(const ufloat& other) const { return value <= other.value; }
    bool operator>=(const ufloat& other) const { return value >= other.value; }

    // Ostream
    friend std::ostream& operator<<(std::ostream& os, const ufloat& f) {
        os << f.value;
        return os;
    }
};

// JSON Serialization
inline void to_json(nlohmann::json& j, const ufloat& f) {
    j = f.value;
}

inline void from_json(const nlohmann::json& j, ufloat& f) {
    f.value = std::max(0.0f, j.get<float>());
}
