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

#ifndef FV1_INSTRUCTION_H_
#define FV1_INSTRUCTION_H_

#include "core/core_type_tag.h"
#include "fv1_defs.h"
#include "fv1_opcodes.h"

namespace fv1 {

static constexpr core::TypeTag kNoneTag = "NONE"_TT;
static constexpr core::TypeTag kValueTag = "VAL"_TT;
static constexpr core::TypeTag kMaskTag = "MASK"_TT;
static constexpr core::TypeTag kRegisterTag = "REG"_TT;
static constexpr core::TypeTag kAddrTag = "ADDR"_TT;

// During parsing, we know what kind of value to extract. So we can annotate the types and
// (optionally) test they are correct when setting up the processing. We could go even further and
// annotate coeff/const as well but this becomes a bit awkward. For now we're assuming everything
// fits in a int32_t and the end user has to figure out how to actually use it based on the type.
//
// NOTE: All fixed point types are converted to SF23 for storage!
// Originally we kept it in the source type format and shifted later; but there didn't seem to be a
// use case for it. We don't lose anything in the conversion since SF23 is sufficient to contain the
// without loss of precision.
//
// The one exception here might be S4F6 but we'll get there eventually.
//
// One might also add a union value, reg, addr, etc. but this becomes complicated and we still end
// up with the user having to check the type.
//
// It'd perhaps be more modern to use std::variant?
struct TypedOperand {
  const core::TypeTag type{kNoneTag};
  const int32_t value{0};

  constexpr TypedOperand() = default;
  constexpr TypedOperand(core::TypeTag t, int32_t v) : type{t}, value{v} {}

  template <typename FP,
            std::enable_if_t<std::is_base_of<core::FixedPointType, FP>::value, bool> = true>
  constexpr explicit TypedOperand(FP v)
      : type{FP::TAG}, value{core::FixedPointConvert<SF23>(v).value}
  {}

  constexpr bool is_none() const { return kNoneTag == type; }
  constexpr bool is_value() const { return kValueTag == type; }
  constexpr bool is_mask() const { return kMaskTag == type; }
  constexpr bool is_register() const { return kRegisterTag == type; }
  constexpr bool is_addr() const { return kAddrTag == type; }

  // Test if contains specific FP type
  template <typename FP>
  constexpr bool is_fixed_point() const
  {
    return core::is_fixed_point_tag<FP>(type);
  }

  // Test if contains any fixed point type
  constexpr bool is_fixed_point() const
  {
    return core::is_fixed_point_tag<S4F6, SF10, S1F9, S1F14, I16, SF15, SF23>(type);
  }

  bool operator!() const { return !value; }
};

// Actual decoded instruction
struct DecodedInstruction {
  using Operands = std::array<TypedOperand, kMaxOperands>;

  const OPCODE opcode_ = OPCODE::UNKNOWN;
  const uint32_t raw = 0;
  const Operands operands = {};
  const size_t num_operands = 0;

  constexpr OPCODE opcode() const { return opcode_; }
};

// Main entry point to decode instructions from binary files.
//
// The actual implementation details aren't really needed here, so \sa fv1_asm_decode.cc
class InstructionDecoder {
public:
  static DecodedInstruction Decode(uint32_t instruction);
};

}  // namespace fv1

#endif  // FV1_INSTRUCTION_H_
