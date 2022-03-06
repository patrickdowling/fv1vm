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

#ifndef ENGINES_DELAY_I32_H_
#define ENGINES_DELAY_I32_H_

#include "fv1/fv1_defs.h"

namespace fv1 {
namespace engine {

struct DelayStorageI32 {
  using value_type = SF23;
  using storage_type = int32_t;

  static inline storage_type Pack(const value_type value) { return value.value; }
  static inline value_type Unpack(const storage_type value) { return value_type{value}; }
};

}  // namespace engine
}  // namespace fv1

#endif  // ENGINES_DELAY_I32_H_
