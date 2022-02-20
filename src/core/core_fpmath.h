// fv1vm: experimental FV-1 virtual machine
// Copyright (C) 2022 Patrick Dowling <pld@gurkenkiste.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef CORE_FPMATH_H_
#define CORE_FPMATH_H_

#include <algorithm>
#include <cstdint>

// These are generic fixed-point ops, intended for use with core::FixedPoint
//
// In some cases there may be optimal target instructions; usually the compile is pretty good about
// that but SSAT seems worth forcing.
//
// MAC (or just *) looks like it could be replaced with SMMLA by (pre-)shifting the coefficient and
// making it a 32x32 multiplication. This works in theory except some of our coefficients are
// S1.FRAC (i.e. go from (-2., 2.] we'd lose a bit.

namespace core {
struct FixedPointType;
template <typename T>
struct FixedPoint;

// This construct isn't great, but detection platform is awkward
#ifdef FV1_MATH_USE_SSAT
template <typename fixed_type>
static inline int32_t SSAT(const int32_t value)
{
  int32_t result;
  __asm volatile("ssat %0, %1, %2" : "=r"(result) : "I"(fixed_type::BITS), "r"(value));
  return result;
}
#else
template <typename fixed_type>
static constexpr inline int32_t SSAT(const int32_t value)
{
  return std::clamp(value, fixed_type::MIN, fixed_type::MAX);
}
#endif

// Sign extend (SBFX)
template <typename T>
static constexpr inline int32_t SX(const int32_t value)
{
  return (value & T::MIN) ? value | T::MIN : value;
}

template <typename FP>
static constexpr inline FP operator*(const FP lhs, const FP rhs)
{
  return FP{static_cast<int32_t>((static_cast<int64_t>(lhs.value) * rhs.value) >> FP::FRAC)};
}

template <typename FP, typename C>
static constexpr inline FP operator*(const FP lhs, const C rhs)
{
  return lhs * static_cast<FP>(rhs);
}

template <typename FP>
static constexpr inline FP operator+(const FP lhs, const FP rhs)
{
  return FP{lhs.value + rhs.value};
}

template <typename FP, typename C>
static constexpr inline FP operator+(const FP lhs, const C rhs)
{
  return lhs + static_cast<FP>(rhs);
}

template <typename FP>
static constexpr inline FP operator-(const FP lhs, const FP rhs)
{
  return FP{lhs.value - rhs.value};
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
static constexpr inline T ABS(const T value)
{
  return value < 0 ? -value : value;
}

template <typename FP, std::enable_if_t<std::is_base_of<FixedPointType, FP>::value, bool> = true>
static constexpr inline FP ABS(const FP fp)
{
  return FP{ABS(fp.value)};
}

template <typename T>
static constexpr bool operator==(const FixedPoint<T> lhs, int32_t value)
{
  return lhs.value == value;
}

template <typename T>
static constexpr bool operator==(const FixedPoint<T> lhs, const FixedPoint<T> rhs)
{
  return lhs.value == rhs.value;
}

template <typename T>
static constexpr bool operator!=(const FixedPoint<T> lhs, const FixedPoint<T> rhs)
{
  return lhs.value != rhs.value;
}

template <typename T>
static constexpr bool operator<(const FixedPoint<T> lhs, int32_t value)
{
  return lhs.value < value;
}

template <typename T>
static constexpr bool operator>(const FixedPoint<T> lhs, const FixedPoint<T> rhs)
{
  return lhs.value > rhs.value;
}

template <typename T>
static constexpr bool operator>=(const FixedPoint<T> lhs, int32_t value)
{
  return lhs.value >= value;
}

// NOTE: logical operations are kept in native integer type for now so we can better distinguish the
// use cases (since we have no implicit conversions).

template <typename T>
static constexpr inline int32_t AND(const int32_t value, const int32_t mask)
{
  return SX<T>(value & mask);
}

template <typename T>
static constexpr inline int32_t OR(const int32_t value, const int32_t mask)
{
  return SX<T>(value | mask);
}

template <typename T>
static constexpr inline int32_t XOR(const int32_t value, const int32_t mask)
{
  return SX<T>(value ^ mask);
}

template <typename T>
static constexpr inline int32_t NOT(const int32_t value)
{
  return SX<T>(~value);
}

}  // namespace core

#endif  // CORE_FPMATH_H_
