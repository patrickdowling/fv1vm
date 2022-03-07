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

#include "fv1_debug.h"

#include <cstdio>

#include "../fv1_instruction.h"
#include "../fv1_opcodes.h"
#include "../fv1_registers.h"

namespace fv1 {
namespace debug {

static const char* const mnemonics[(int)OPCODE::UNKNOWN + 1] = {
    "rda",
    "rmpa",
    "wra",
    "wrap",
    "rdax",
    "rdfx",
    "wrax",
    "wrhx",
    "wrlx",
    "maxx",
    "mulx",
    "log",
    "exp",
    "sof",
    "and",
    "or",
    "xor",
    "skp",
    "wlds",
    "jam",
    "cho rda",  // 0x14
    "real_opcodes_last",
    "0x16",
    "0x17",
    "0x18",
    "0x19",
    "0x1a",
    "0x1b",
    "0x1c",
    "0x1d",
    "0x1e",
    "0x1f",
    "clr",  // 0x20
    "not",
    "absa",
    "ldax",
    "wldr",
    "cho rdal",
    "cho sof",
    "cho sof rmp",
    "cho sof sin",
    "cho rda rmp",
    "cho rda sin",
    "jmp",
    "nop",
    "UNKNOWN",
};

const char* to_string(OPCODE opcode)
{
  return mnemonics[static_cast<uint8_t>(opcode)];
}

const char* register_names[REGISTER::LAST] = {
    "SIN0_RATE",  "SIN0_RANGE", "SIN1_RATE", "SIN1_RANGE", "RMP0_RATE", "RMP0_RANGE", "RMP1_RATE",
    "RMP1_RANGE", "0x8",        "0x9",       "0xa",        "0xb",       "0xc",        "0xd",
    "0xe",        "0xf",
    "POT0",  // = 0x10 = 16
    "POT1",       "POT2",       "0x13",      "ADCL",       "ADCR",      "DACL",       "DACR",
    "ADDR_PTR",   "0x19",       "0x1a",      "0x1b",       "0x1c",      "0x1d",       "0x1e",
    "0x1f",
    "REG0",  // 0x20 = 32
    "REG1",       "REG2",       "REG3",      "REG4",       "REG5",      "REG6",       "REG7",
    "REG8",       "REG9",       "REG10",     "REG11",      "REG12",     "REG13",      "REG14",
    "REG15",      "REG16",      "REG17",     "REG18",      "REG19",     "REG20",      "REG21",
    "REG22",      "REG23",      "REG24",     "REG25",      "REG26",     "REG27",      "REG28",
    "REG29",      "REG30",      "REG31",
};

const char* to_string(REGISTER reg)
{
  return register_names[reg];
}

FlagString to_string(CHO_FLAGS cho_flags)
{
  FlagString flag_string;
  auto s = flag_string.string;
  if (CHO_FLAGS::COS & cho_flags) s += sprintf(s, "COS ");
  if (CHO_FLAGS::REG & cho_flags) s += sprintf(s, "REG ");
  if (CHO_FLAGS::COMPC & cho_flags) s += sprintf(s, "COMPC ");
  if (CHO_FLAGS::COMPA & cho_flags) s += sprintf(s, "COMPA ");
  if (CHO_FLAGS::RPTR2 & cho_flags) s += sprintf(s, "RPTR2 ");
  if (CHO_FLAGS::NA & cho_flags) s += sprintf(s, "NA ");
  return flag_string;
}

int print_operand(char* buf, const TypedOperand& operand)
{
  if (operand.is_register())
    return sprintf(buf, "%s", to_string(static_cast<REGISTER>(operand.value)));
  else if (operand.is_addr()) {
    return sprintf(buf, "[%04x]", operand.value);
  } else if (operand.is_value() || operand.is_fixed_point<I16>()) {
    return sprintf(buf, "%d", operand.value);
  } else if (operand.is_mask()) {
    return sprintf(buf, "%#x", operand.value);
  } else if (!operand.is_none()) {
    return sprintf(buf, "%#f", (double)SF23{operand.value}.to_float());
  }
  return 0;
}

int print_operand_details(char* buf, const TypedOperand& operand)
{
  if (operand.is_value()) return sprintf(buf, "%#06x", operand.value);
  if (operand.is_register()) return sprintf(buf, "%02d", operand.value);
  if (operand.is_addr() || operand.is_mask()) return sprintf(buf, "%s", operand.type.str().value);
  return sprintf(buf, "%s:%#x", operand.type.str().value, operand.value);
}

void print_decoded_instruction(int ic, const DecodedInstruction& di, bool print_binary /*= false*/)
{
  char buf[128] = {0};
  auto p = buf;

  p += sprintf(p, "[%03d] %-4s", ic, to_string(di.opcode()));
  char sep = ' ';
  for (size_t i = 0; i < di.num_operands; ++i) {
    p += sprintf(p, "%c", sep);
    sep = ',';
    p += print_operand(p, di.operands[i]);
  }

  printf("%-40s", buf);

  p = buf;
  if (print_binary)
    p += sprintf(p, " ; [%08x] %02x", di.raw, (unsigned)di.opcode());
  else
    p += sprintf(p, " ; %02x", (unsigned)di.opcode());
  sep = ' ';
  for (size_t i = 0; i < di.num_operands; ++i) {
    p += sprintf(p, "%c", sep);
    sep = ',';
    p += print_operand_details(p, di.operands[i]);
  }
  printf("%s\n", buf);
}

}  // namespace debug
}  // namespace fv1
