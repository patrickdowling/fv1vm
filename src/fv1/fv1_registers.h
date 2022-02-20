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

#ifndef FV1_REGISTERS_H_
#define FV1_REGISTERS_H_

#include <cstdint>

#include "fv1_defs.h"

namespace fv1 {

enum REGISTER : uint8_t {
  SIN0_RATE = 0x00,
  SIN0_RANGE,
  SIN1_RATE,
  SIN1_RANGE,
  RMP0_RATE,
  RMP0_RANGE,
  RMP1_RATE,
  RMP1_RANGE,
  POT0 = 0x10,
  POT1,
  POT2,
  REG13h,
  ADCL,
  ADCR,
  DACL,
  DACR,
  ADDR_PTR = 0x18,  // 24, 25-31 not used
  REG0 = 0x20,      // 32-63
  REG1,
  REG2,
  REG3,
  REG4,
  REG5,
  REG6,
  REG7,
  REG8,
  REG9,
  REG10,
  REG11,
  REG12,
  REG13,
  REG14,
  REG15,
  REG16,
  REG17,
  REG18,
  REG19,
  REG20,
  REG21,
  REG22,
  REG23,
  REG24,
  REG25,
  REG26,
  REG27,
  REG28,
  REG29,
  REG30,
  REG31,
  LAST
};
static_assert(0x10 == static_cast<uint8_t>(REGISTER::POT0));
static_assert(0x14 == static_cast<uint8_t>(REGISTER::ADCL));
static_assert(0x18 == static_cast<uint8_t>(REGISTER::ADDR_PTR));
static_assert(0x3f == static_cast<uint8_t>(REGISTER::REG31));
static constexpr size_t kNumRegisters = static_cast<size_t>(REGISTER::LAST);

}  // namespace fv1

#endif  // FV1_REGISTERS_H_
