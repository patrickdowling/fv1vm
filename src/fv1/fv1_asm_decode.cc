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

#include "fv1_asm_decode.h"

#include <algorithm>
#include <type_traits>

#include "core/core_templates.h"

#define FV1_INSTRUCTION(name, bitfields, ...)                                          \
  struct name : public InstructionImpl<name, OPCODE::name, __VA_ARGS__> {              \
    static constexpr std::string_view STRING = bitfields;                              \
    static_assert(STRING.size() == 32);                                                \
    static constexpr OpcodeMatcher kOpcodeMatcher = OpcodeMatcher::FromString<name>(); \
    static_assert((kOpcodeMatcher.mask & 0x1f) == 0x1f);                               \
  }

namespace fv1 {
namespace detail {

// TODO Can we pack min/max value here? It's rarely actually needed
// TODO Inconsistency in I16: In one case we want a integer (WLDR), the other a real (CHO_SOF)
// clang-format off
FV1_INSTRUCTION(RDA,  "CCCCCCCCCCCAAAAAAAAAAAAAAAA00000", ADDR<'A'>, REAL<S1F9, 'C'>);
FV1_INSTRUCTION(RMPA, "CCCCCCCCCCC000000000001100000001", REAL<S1F9, 'C'>);
FV1_INSTRUCTION(WRA,  "CCCCCCCCCCCAAAAAAAAAAAAAAAA00010", ADDR<'A'>, REAL<S1F9, 'C'>);
FV1_INSTRUCTION(WRAP, "CCCCCCCCCCCAAAAAAAAAAAAAAAA00011", ADDR<'A'>, REAL<S1F9, 'C'>);
FV1_INSTRUCTION(RDAX, "CCCCCCCCCCCCCCCC00000AAAAAA00100", REGS<'A'>, REAL<S1F14, 'C'>);
FV1_INSTRUCTION(RDFX, "CCCCCCCCCCCCCCCC00000AAAAAA00101", REGS<'A'>, REAL<S1F14, 'C'>);
FV1_INSTRUCTION(WRAX, "CCCCCCCCCCCCCCCC00000AAAAAA00110", REGS<'A'>, REAL<S1F14, 'C'>);
FV1_INSTRUCTION(WRHX, "CCCCCCCCCCCCCCCC00000AAAAAA00111", REGS<'A'>, REAL<S1F14, 'C'>);
FV1_INSTRUCTION(WRLX, "CCCCCCCCCCCCCCCC00000AAAAAA01000", REGS<'A'>, REAL<S1F14, 'C'>);
FV1_INSTRUCTION(MAXX, "CCCCCCCCCCCCCCCC00000AAAAAA01001", REGS<'A'>, REAL<S1F14, 'C'>);
FV1_INSTRUCTION(MULX, "000000000000000000000AAAAAA01010", REGS<'A'>);
FV1_INSTRUCTION(LOG,  "CCCCCCCCCCCCCCCCDDDDDDDDDDD01011", REAL<S1F14, 'C'>, REAL<SF10, 'D'>); // S4F6?
FV1_INSTRUCTION(EXP,  "CCCCCCCCCCCCCCCCDDDDDDDDDDD01100", REAL<S1F14, 'C'>, REAL<SF10, 'D'>);
FV1_INSTRUCTION(SOF,  "CCCCCCCCCCCCCCCCDDDDDDDDDDD01101", REAL<S1F14, 'C'>, REAL<SF10, 'D'>);
FV1_INSTRUCTION(AND,  "MMMMMMMMMMMMMMMMMMMMMMMM00001110", MASK<'M'>);
FV1_INSTRUCTION(OR,   "MMMMMMMMMMMMMMMMMMMMMMMM00001111", MASK<'M'>);
FV1_INSTRUCTION(XOR,  "MMMMMMMMMMMMMMMMMMMMMMMM00010000", MASK<'M'>);
FV1_INSTRUCTION(SKP,  "CCCCCNNNNNN000000000000000010001", MASK<'C', 5>, INT<'N', 6>);
FV1_INSTRUCTION(WLDS, "00NFFFFFFFFFAAAAAAAAAAAAAAA10010", INT<'N', 1>, INT<'F', 9>, INT<'A', 15>);
FV1_INSTRUCTION(WLDR, "01NFFFFFFFFFFFFFFFF000000AA10010", INT<'N', 1>, REAL<I16, 'F'>, INT<'A', 2>);
FV1_INSTRUCTION(JAM,  "0000000000000000000000001N010011", INT<'N', 1>);
FV1_INSTRUCTION(CHO_RDA,  "00CCCCCC0NNAAAAAAAAAAAAAAAA10100", INT<'N', 2>, INT<'C', 6>, ADDR<'A'>);
FV1_INSTRUCTION(CHO_SOF,  "10CCCCCC0NNDDDDDDDDDDDDDDDD10100", INT<'N', 2>, INT<'C', 6>, REAL<I16, 'D'>);
FV1_INSTRUCTION(CHO_RDAL, "11CCCCCC0NN000000000000000010100", INT<'N', 2>, INT<'C', 6>); // see notes
// clang-format on
//
// NOTE CHO_RDAL is includes flags which is somewhat glossed over in asm manual
//      We really only need the COS flag?
//      See https://github.com/ndf-zz/asfv1#cho-rdal-lfo--flags

// The "plain" opcodes are stored at the beginning of the table for immediate lookup in slots
// [0, kOpcodeMax). Any ones that need disambiguation are stored at the the end and searched
// (linearly). So the general case is simple, and there are fewer rare cases so it doesn't seem
// like a problem.
using DecoderFunction = DecodedInstruction (*)(uint32_t);
static constexpr uint32_t table_index(uint32_t value)
{
  return value & 0x1fU;
}

struct TableEntry {
  OpcodeMatcher matcher = {};
  DecoderFunction decoder_fn = nullptr;
};

// This is a bit wasteful (there are 21-ish operands, not 0x1f) but in case of unknown operands, we
// end up with a null decoding function.
using InstructionTable = std::array<TableEntry, kOpcodeMax + kNumSecondaryOpcodes>;

template <typename Instruction>
constexpr void RegisterInstruction(InstructionTable &table)
{
  constexpr auto matcher = Instruction::kOpcodeMatcher;
  static_assert(matcher.primary() < kOpcodeMax);
  size_t slot = 0;
  if constexpr (!matcher.secondary()) {
    slot = matcher.primary();
  } else {
    slot = kOpcodeMax;
    while (table[slot].decoder_fn) ++slot;
  }

  table[slot].matcher = matcher;
  table[slot].decoder_fn = Instruction::Decode;
}

template <typename... Instructions>
static constexpr InstructionTable BuildInstructionTable()
{
  static_assert(core::are_distinct_types<Instructions...>::value);

  InstructionTable instruction_table;
  (RegisterInstruction<Instructions>(instruction_table), ...);
  return instruction_table;
}

static constexpr InstructionTable instruction_table_ =
    BuildInstructionTable<RDA, RMPA, WRA, WRAP, RDAX, RDFX, WRAX, WRHX, WRLX, MAXX, MULX, LOG, EXP,
                          SOF, AND, OR, XOR, SKP, WLDS, WLDR, JAM, CHO_RDA, CHO_SOF, CHO_RDAL>();

}  // namespace detail

// Main entry point for decoding instructions using a table to lookup the decoding function for the
// instruction opcode; Adds a whole bunch of hoops for the WLDR/WLDS and CHO_* that share an opcode,
// but have different operand layouts. These are identifiable by the top two bits which we can use
// as a secondary index.
//
// With some additional hoops the actual lookup table becomes constexpr. The downside is that we
// have to list the instructions twice. We can _maybe_ get around that but passing any type of
// string as a template parameter is hard/weird.
/*static*/ DecodedInstruction InstructionDecoder::Decode(uint32_t instruction)
{
  detail::DecoderFunction decoder_fn = nullptr;

  using detail::instruction_table_;

  const auto &table_entry = instruction_table_[detail::table_index(instruction)];
  if (!table_entry.matcher.has_secondary() || table_entry.matcher.match(instruction)) {
    decoder_fn = table_entry.decoder_fn;
  } else {
    auto ste = std::find_if(
        instruction_table_.begin() + kOpcodeMax, instruction_table_.end(),
        [instruction](const detail::TableEntry &te) { return te.matcher.match(instruction); });
    if (instruction_table_.end() != ste) decoder_fn = ste->decoder_fn;
  }

  if (decoder_fn) {
    return decoder_fn(instruction);
  } else {
    return {OPCODE::UNKNOWN, instruction};
  }
}

}  // namespace fv1
