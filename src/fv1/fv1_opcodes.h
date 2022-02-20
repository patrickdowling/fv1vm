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

#ifndef FV1_OPCODES_H_
#define FV1_OPCODES_H_

#include <cstdint>
#include <cstring>

namespace fv1 {

enum class OPCODE : uint8_t {
  RDA = 0x00,
  RMPA = 0x01,
  WRA = 0x02,
  WRAP = 0x03,
  RDAX = 0x04,
  RDFX = 0x05,  // => LDAX
  WRAX = 0x06,
  WRHX = 0x07,
  WRLX = 0x08,
  MAXX = 0x09,  // => ABSA
  MULX = 0x0a,
  LOG = 0x0b,
  EXP = 0x0c,
  SOF = 0x0d,
  AND = 0x0e,  // CLR
  OR = 0x0f,
  XOR = 0x10,   // NOT
  SKP = 0x11,   // NOP
  WLDS = 0x12,  // => WLDR
  JAM = 0x13,
  CHO_RDA = 0x14,  // => CHO_SOF, CHO_RDAL
  REAL_OPCODES_LAST,
  // ALT/PSEUDO OPCODES (start with 0x2.)
  // TODO we might also use 0x20 | original opcode but that fails with CHO_RDA and NOP
  CLR = 0x20,
  NOT,
  ABSA,
  LDAX,
  WLDR,  // 0x40000012, see WLDS
  CHO_RDAL,
  CHO_SOF,      // Actually we can probably split these further depending on flags
  CHO_SOF_RMP,  // VM
  CHO_SOF_SIN,  // VM
  CHO_RDA_RMP,  // VM
  CHO_RDA_SIN,  // VM
  JMP,          // SKP with no flags but N > 0
  NOP,          // SKP with not flags and N == 0
  UNKNOWN,
};
static constexpr size_t kNumRealOpcodes = static_cast<size_t>(OPCODE::REAL_OPCODES_LAST);
static constexpr size_t kNumOpcodes = static_cast<size_t>(OPCODE::UNKNOWN) + 1;
static constexpr size_t kNumSecondaryOpcodes = 2 /*WLDS/R*/ + 3 /*CHO_*/;

enum SKP_FLAGS : int32_t {
  NEG = 0x01,  // If ACC is negative
  GEZ = 0x02,  // If ACC is greater than or equal to zero
  ZRO = 0x04,  // If ACC is zero
  ZRC = 0x08,  // If the sign of ACC and PACC are different
  RUN = 0x10,  // Set after first program execution
};

enum CHO_SEL : int32_t {
  SIN0 = 0x00,
  SIN1 = 0x01,
  RMP0 = 0x02,
  RMP1 = 0x03,
};
static constexpr int32_t CHO_SEL_MASK = 0x3;

enum CHO_FLAGS : int32_t {
  SIN = 0x00,
  COS = 0x01,
  REG = 0x02,
  COMPC = 0x04,
  COMPA = 0x08,
  RPTR2 = 0x10,
  NA = 0x20,
};

}  // namespace fv1

#endif  // FV1_OPCODES_H_
