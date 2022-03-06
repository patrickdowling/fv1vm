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

#ifndef FV1_VM_TYPES_H_
#define FV1_VM_TYPES_H_

#include <cstdint>
#include <type_traits>

#include "fv1/fv1_defs.h"

namespace fv1 {

template <typename T>
struct AudioFrameT {
  T l = 0;
  T r = 0;
};

template <typename T>
struct ParametersT {
  T pots[kNumPots] = {0, 0, 0};
};

// Basic interface for register types
template <typename T>
struct RegisterBase {
public:
  void clr() { static_cast<T *>(this)->store(T::ZERO); }
  inline bool neg() const { return static_cast<const T *>(this)->load() < T::ZERO; }
  inline bool gez() const { return static_cast<const T *>(this)->load() >= T::ZERO; }
  inline bool zero() const { return static_cast<const T *>(this)->load() == T::ZERO; }

  inline int32_t load_addr() const
  {
    return (static_cast<const T *>(this)->loadi() >> 8) & kDelayAddrMask;
  }
};

// Seems like we want to support a union of int32_t/float.
// Alternatively, use a hardcore cast/memcpy to get value from "raw" which generally gets optimized
// away anyway.

// For debug/testing it seems useful to add a type flag field as well.
// Then whoever is asking can check for the right field type (perhaps not as good as a compile-time
// version, but still).

template <typename T>
struct ConstantBase {
  // TypeTag type_tag;
};

template <typename T>
struct IntegerConstantT {
  explicit IntegerConstantT(const T &source) : source_{source} {}

  operator int32_t() const { return source_.loadi(); }

  template <typename enum_type>
  enum_type enum_cast() const
  {
    static_assert(std::is_enum<enum_type>::value);
    return static_cast<enum_type>(source_.loadi());
  }

  template <typename enum_type, int32_t mask>
  enum_type enum_cast() const
  {
    static_assert(std::is_enum<enum_type>::value);
    return static_cast<enum_type>(source_.loadi() & mask);
  }

private:
  const T &source_;
};

template <typename T>
struct IndexConstantT {
  explicit IndexConstantT(const T &source) : source_{source} {}

  operator uint32_t() const { return static_cast<uint32_t>(source_.loadi()); }

private:
  const T &source_;
};

template <typename T>
struct FloatConstantT {
  explicit FloatConstantT(const T &source) : source_{source} {}

  operator const typename T::float_value() const { return source_.load(); }

private:
  const T &source_;
};

}  // namespace fv1

#endif  // FV1_VM_TYPES_H_
