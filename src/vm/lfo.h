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

#ifndef FV1_LFO_H_
#define FV1_LFO_H_

#include "vm_types.h"

namespace fv1 {

// Base for LFO features. We're not using them polymorphically.
template <typename Engine>
class LfoBase {
public:
  LfoBase() = delete;

  // Helper for returning values from LFOs
  struct LfoValue {
    explicit LfoValue(const int32_t off, const typename Engine::float_type coeff)
        : offset(off), coefficient(coeff)
    {}

    const int32_t offset;                           // adjusted for LFO range
    const typename Engine::float_type coefficient;  // [0, 1)
  };

protected:
  LfoBase(const typename Engine::Register *rate, const typename Engine::Register *range)
      : rate_{rate}, range_{range}
  {}

  const typename Engine::Register *rate_;
  const typename Engine::Register *range_;
};

}  // namespace fv1

#endif  // FV1_LFO_H_
