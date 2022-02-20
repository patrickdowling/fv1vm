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

namespace fv1 {

// TODO Make InstructionDecoder::instruction_table_, secondary_table_ constexpr?
// TODO Can we pack min/max value here? It's rarely actually needed
// TODO Inconsistency in I16: In one case we want a integer (WLDR), the other a real (CHO_SOF)

// clang-format off
FV1_INSTRUCTIONS_BEGIN()
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
FV1_INSTRUCTIONS_END()
// clang-format on
//
// NOTE CHO_RDAL is includes flags which is somewhat glossed over in asm manual
//      We really only need the COS flag?
//      See https://github.com/ndf-zz/asfv1#cho-rdal-lfo--flags

/*static*/ void InstructionDecoder::Register(const OpcodeMatcher &matcher, DecoderFunction fn)
{
  // printf("** mask=%08x pattern=%08x p=%08x s=%08x ", matcher.mask, matcher.pattern,
  // matcher.primary(), matcher.secondary());
  if (matcher.secondary()) {
    auto table_entry = std::find_if_not(secondary_table_.begin(), secondary_table_.end(),
                                        [](const TableEntry &te) { return te.decoder_fn; });
    table_entry->matcher = matcher;
    table_entry->decoder_fn = fn;
  } else {
    auto &table_entry = instruction_table_[table_index(matcher.pattern)];
    table_entry.matcher = matcher;
    table_entry.decoder_fn = fn;
  }
}

/*static*/ DecodedInstruction InstructionDecoder::Decode(uint32_t instruction)
{
  DecoderFunction decoder_fn = nullptr;

  const auto &table_entry = instruction_table_[table_index(instruction)];
  if (!table_entry.matcher.has_secondary() || table_entry.matcher.match(instruction)) {
    decoder_fn = table_entry.decoder_fn;
  } else {
    auto ste =
        std::find_if(secondary_table_.begin(), secondary_table_.end(),
                     [instruction](const TableEntry &te) { return te.matcher.match(instruction); });
    if (secondary_table_.end() != ste) decoder_fn = ste->decoder_fn;
  }

  if (decoder_fn) {
    return decoder_fn(instruction);
  } else {
    return {OPCODE::UNKNOWN, instruction};
  }
}

}  // namespace fv1
