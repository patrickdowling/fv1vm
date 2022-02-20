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

#ifndef FV1_DEBUG_H_
#define FV1_DEBUG_H_

#include "../fv1_defs.h"

// This should probably only be enabled in non-embedded builds

namespace fv1 {

enum REGISTER : uint8_t;
enum CHO_FLAGS : int32_t;
enum class OPCODE : uint8_t;
enum class OPERAND_TYPE;
struct DecodedInstruction;
namespace debug {

const char *to_string(OPCODE opcode);
const char *to_string(REGISTER reg);

// Return string on the stack
struct FlagString {
  char string[64] = {0};
};
FlagString to_string(CHO_FLAGS cho_flags);

void print_decoded_instruction(int ic, const DecodedInstruction &di, bool print_binary = false);

}  // namespace debug
}  // namespace fv1

#endif // FV1_DEBUG_H_
