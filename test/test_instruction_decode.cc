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

#include <gtest/gtest.h>

#include "fv1/debug/fv1_debug.h"
#include "fv1/fv1_asm_decode.h"

namespace testfv1 {

struct TestOpcodeParam {
  const uint32_t binary;
  const fv1::OPCODE opcode;
};

std::ostream &operator<<(std::ostream &os, const TestOpcodeParam &param)
{
  char buffer[128] = {0};
  sprintf(buffer, "0x%08x=%s", param.binary, fv1::debug::to_string(param.opcode));
  os << buffer;
  return os;
}

class TestOpcodeMatchers : public ::testing::TestWithParam<TestOpcodeParam> {};

TEST_P(TestOpcodeMatchers, Match)
{
  auto opcode = GetParam();
  auto decoded_instruction = fv1::InstructionDecoder::Decode(opcode.binary);

  EXPECT_EQ(opcode.opcode, decoded_instruction.opcode());
}

static constexpr TestOpcodeParam opcodes[] = {
    {0b01101, fv1::OPCODE::SOF},
    {0b01110, fv1::OPCODE::AND},
    {0b01111, fv1::OPCODE::OR},
    {0b10000, fv1::OPCODE::XOR},
    {0b01011, fv1::OPCODE::LOG},
    {0b01100, fv1::OPCODE::EXP},
    {0b10001, fv1::OPCODE::SKP},
    {0b00100, fv1::OPCODE::RDAX},
    {0b00110, fv1::OPCODE::WRAX},
    {0b01001, fv1::OPCODE::MAXX},
    {0b01010, fv1::OPCODE::MULX},
    {0b00101, fv1::OPCODE::RDFX},
    {0b01000, fv1::OPCODE::WRLX},
    {0b00111, fv1::OPCODE::WRHX},
    {0b00000, fv1::OPCODE::RDA},
    {0b00001, fv1::OPCODE::RMPA},
    {0b00010, fv1::OPCODE::WRA},
    {0b00011, fv1::OPCODE::WRAP},
    {0x00000000 | 0b10010, fv1::OPCODE::WLDS},
    {0x40000000 | 0b10010, fv1::OPCODE::WLDR},
    {0b10011, fv1::OPCODE::JAM},
    {0x00000000 | 0b10100, fv1::OPCODE::CHO_RDA},
    {0x80000000 | 0b10100, fv1::OPCODE::CHO_SOF},
    {0xC0000000 | 0b10100, fv1::OPCODE::CHO_RDAL},
    {0x1f, fv1::OPCODE::UNKNOWN},
    {0xffffffff, fv1::OPCODE::UNKNOWN},
};

INSTANTIATE_TEST_SUITE_P(, TestOpcodeMatchers, testing::ValuesIn(opcodes),
                         [](const testing::TestParamInfo<TestOpcodeParam> &tpi) {
                           char buffer[128] = {0};
                           sprintf(buffer, "%s_%08x", fv1::debug::to_string(tpi.param.opcode),
                                   tpi.param.binary);
                           std::string str = buffer;
                           std::replace(str.begin(), str.end(), ' ', '_');
                           return str;
                         });

}  // namespace testfv1
