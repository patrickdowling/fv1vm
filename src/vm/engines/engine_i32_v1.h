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

#ifndef ENGINES_ENGINE_I32_H_
#define ENGINES_ENGINE_I32_H_

#include <cstdint>
#include <type_traits>

#include "fv1/fv1_defs.h"
#include "vm/vm_types.h"

// Very basic int32_t/SF23 implementation
namespace fv1 {
namespace engine {

struct EngineI32 {
  using float_type = int32_t;
  using float_value = SF23;

  static constexpr float_type ONE = SF23::MAX;

  struct Register : public RegisterBase<Register> {
    static constexpr int32_t ZERO = 0;

    float_value load() const { return float_value{value}; }
    int32_t loadi() const { return value; }

    void store(const Register &r) { value = r.value; }
    void store(const float_type v) { value = core::SSAT<SF23>(v); }
    void store(const float_value v) { value = core::SSAT<SF23>(v.value); }

    void storei(int32_t v) { value = v; }

    // We're in a bit of pickle for the audio frames
    void read(float_type &v) const { v = value; }

  private:
    int32_t value{0};
  };

  struct Constant : public ConstantBase<Constant> {
    using float_value = fv1::SF23;

    float_value load() const { return SF23{value}; }
    int32_t loadi() const { return value; }

    inline void store(const int32_t v) { value = v; }
    inline void store(const SF23 v) { value = v.value; }

    bool zero() const { return !value; }

  private:
    int32_t value{0};
  };

  static constexpr SF23 ABS(const SF23 value) { return core::ABS(value); }

  template <int32_t bits>
  static constexpr float_type LfoCoeffToFloat(int32_t f)
  {
    return f << (SF23::BITS - bits);
  }
};

}  // namespace engine
}  // namespace fv1

#endif  // ENGINES_ENGINE_I32_H_
