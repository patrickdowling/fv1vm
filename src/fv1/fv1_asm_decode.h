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

#ifndef FV1_ASM_DECODE_H_
#define FV1_ASM_DECODE_H_

#include <array>
#include <string_view>

#include "fv1_defs.h"
#include "fv1_instruction.h"

// All this is mainly infrastructure be be able to parse binary instructions.
// Given the number of actual instructions this is a lot of boilerplate compared to just doing it
// brute force would be but if nothing else, it's an interesting experiment in constexpr :)
//
// The vague idea is the use the definitions that come in SPINAsmUserManual to extract
// the operands without having to explicity write out length/shifts. The definitions look some thing
// like CCCCCNNNNNN000000000000000010001 so the desired fields are indicated with 'C' and 'N'. We
// still manually define the _type_ of operand required but we can at least plausibly check if the
// size matches.
//
// Each instruction generates a ::Decode function, and there's a mapping from opcode -> Decode.
// This mapping is has two layers to cater for ambiguous opcodes.
//
// NOTE ARM has extra instructions to extract bits from registers

namespace fv1 {
namespace detail {

struct BitField {
  const size_t width;  // Number of bits
  const size_t shift;  // Number of bits to shift

  template <char ID, typename T>
  static constexpr BitField FromString()
  {
    static_assert(T::STRING.length() == 32);
    auto first = T::STRING.find(ID);
    auto last = T::STRING.find_last_of(ID);
    if (std::string_view::npos != last)
      return {last - first + 1, T::STRING.length() - 1 - last};
    else
      return {0, 0};
  }

  constexpr bool valid() const { return width > 0; }
  constexpr uint32_t Read(uint32_t value) const { return (value >> shift) & ((0x1U << width) - 1); }
};

// int32_t types
template <core::Tag operand_type, char id, size_t expected_width>
struct IntOperand {
  static constexpr char ID = id;
  static constexpr TypedOperand Read(uint32_t value)
  {
    return {core::TypeTag{operand_type}, static_cast<int32_t>(value)};
  }
  static constexpr bool valid_width(size_t w) { return expected_width == w; }
};

// FP types
template <typename T, char id>
struct FixedOperand {
  static constexpr char ID = id;
  static constexpr TypedOperand Read(uint32_t value)
  {
    return TypedOperand{core::DecodeFixedPoint<T>(value)};
  }

  static constexpr bool valid_width(size_t w) { return T::BITS == w; }
};

template <char id, size_t width>
using INT = IntOperand<kValueTag.tag, id, width>;
template <char id, size_t width = 24>
using MASK = IntOperand<kMaskTag.tag, id, width>;
template <char id>
using REGS = IntOperand<kRegisterTag.tag, id, 6>;
template <char id>
using ADDR = IntOperand<kAddrTag.tag, id, 1 + 15>;
template <typename T, char id>
using REAL = FixedOperand<T, id>;
template <typename T, char id>
using DEC = FixedOperand<T, id>;  // This is just syntactic sugar

struct OpcodeMatcher {
  uint32_t mask = 0;
  uint32_t pattern = 0;

  // For instruction differentiation (which is for WLDR/WLDR and CHO_XXXX) it looks like it's
  // enough to ignore the middle bits(Some of the definitions contain additional 1s).
  static constexpr uint32_t kPrimaryKeyMask = 0x0000001f;
  static constexpr uint32_t kSecondaryKeyMask = 0xc0000000;

  template <typename T>
  static constexpr OpcodeMatcher FromString()
  {
    uint32_t mask = 0;
    uint32_t pattern = 0;
    uint32_t bits = 0x80000000;
    for (auto c : T::STRING) {
      if ('1' == c) pattern |= bits;
      if ('1' == c || '0' == c) mask |= bits;
      bits >>= 1;
    }
    mask &= (kPrimaryKeyMask | kSecondaryKeyMask);
    pattern &= (kPrimaryKeyMask | kSecondaryKeyMask);
    return {mask, pattern};
  }

  constexpr uint32_t primary() const { return pattern & kPrimaryKeyMask; }
  constexpr uint32_t secondary() const { return pattern >> 30; }
  constexpr bool has_secondary() const { return mask & kSecondaryKeyMask; }

  constexpr bool match(uint32_t value) const { return (value & mask) == pattern; }

  constexpr bool match_primary(uint32_t value) const
  {
    return (value & mask & kPrimaryKeyMask) == (pattern & kPrimaryKeyMask);
  }
};

// The structs aren't _really_ meant to be instantiated.
template <typename T, typename... operand_types>
struct InstructionImpl {
  static constexpr size_t kNumOperands = sizeof...(operand_types);
  static_assert(kNumOperands <= 4);

  template <typename Operand>
  static constexpr auto DecodeOperand(uint32_t instruction)
  {
    constexpr auto bitfield = BitField::FromString<Operand::ID, T>();
    static_assert(bitfield.valid());
    static_assert(Operand::valid_width(bitfield.width));
    return Operand::Read(bitfield.Read(instruction));
  }

  static constexpr DecodedInstruction Decode(uint32_t instruction)
  {
    return {T::kOpcode, instruction, {DecodeOperand<operand_types>(instruction)...}, kNumOperands};
  }
};

}  // namespace detail
}  // namespace fv1

#endif  // FV1_ASM_DECODE_H_
