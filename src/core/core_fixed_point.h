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

#ifndef CORE_FIXED_POINT_H_
#define CORE_FIXED_POINT_H_

#include <cstdint>
#include <type_traits>

#include "core/core_fpmath.h"
#include "core/core_type_tag.h"

namespace core {

#define FPS_(x, y) x##y
#define FPS(x) FPS_(#x, _TTU)
#define FIXED_POINT(x, bits, frac) \
  using x = core::FixedPoint<core::FixedPointTraits<FPS(x), bits, frac>>

#define FIXED_POINT_S(x, s, bits, frac) \
  using x = core::FixedPoint<core::FixedPointTraits<FPS_(s, _TTU), bits, frac>>

// Common ancestor for fixed point implementations (e.g. for enable_if matching)
// NOTE putting ::value in here seems logical enough, but apparenty slower?
struct FixedPointType {};

template <typename T>
constexpr bool is_fixed_point()
{
  return std::is_base_of_v<FixedPointType, T>;
}

template <typename... FPs>
constexpr bool is_fixed_point_tag(const core::TypeTag tag)
{
  static_assert((... && is_fixed_point<FPs>()));
  return (... || (tag == FPs::TAG));
}

// To define a fixed-point type, use this
template <uint32_t type_tag, uint32_t bits, uint32_t frac>
struct FixedPointTraits {
  static constexpr core::TypeTag TAG = core::TypeTag{type_tag};
  static constexpr uint32_t BITS = bits;
  static constexpr uint32_t FRAC = frac;
};

// Signed fixed point helper
// This is intended for on-the-fly construction of variables so it *doesn't* SSAT on construction.
// The Decode function however does.
//
// The float conversions normalize the values to the native range (e.g. (-1,1]).
//
template <typename Traits>
struct FixedPoint : public FixedPointType {
  static_assert(Traits::BITS > Traits::FRAC);
  static_assert(Traits::BITS > 1);
  static_assert(Traits::BITS < 32);

  static constexpr core::TypeTag TAG = Traits::TAG;

  static constexpr uint32_t BITS = Traits::BITS;
  static constexpr uint32_t FRAC = Traits::FRAC;

  static constexpr int32_t kIntRange = 1 << (Traits::BITS - 1);  // account for sign
  static constexpr float kFloatScale = 1 << Traits::FRAC;

  static constexpr int32_t MAX = kIntRange - 1;
  static constexpr int32_t MIN = -kIntRange;
  static constexpr int32_t MASK = (1 << Traits::BITS) - 1;

  constexpr FixedPoint() = default;
  constexpr explicit FixedPoint(int32_t v) : value{v} {}

  constexpr float to_float() const { return static_cast<float>(value) / kFloatScale; }

  int32_t value{0};
};

template <typename dest_type, typename source_type>
static constexpr dest_type FixedPointConvert(const source_type src)
{
  static_assert(is_fixed_point<dest_type>());
  static_assert(is_fixed_point<source_type>());
  auto v = src.value;
  if constexpr (dest_type::FRAC > source_type::FRAC)
    v <<= (dest_type::FRAC - source_type::FRAC);
  else
    v >>= (source_type::FRAC - dest_type::FRAC);

  return dest_type{v};
}

template <typename fixed_type, bool ssat = true>
static constexpr fixed_type FixedFromFloat(const float f32)
{
  auto i = static_cast<int32_t>(f32 * fixed_type::kFloatScale);
  if constexpr (ssat)
    return fixed_type{SSAT<fixed_type>(i)};
  else
    return fixed_type{i};
}

template <typename fixed_type>
static constexpr fixed_type DecodeFixedPoint(uint32_t raw)
{
  int32_t i = SX<fixed_type>(static_cast<int32_t>(raw));
  return fixed_type{SSAT<fixed_type>(i)};
}

}  // namespace core

#endif  // CORE_FIXED_POINT_H_
