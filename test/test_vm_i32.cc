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

#include "test_vm.h"
#include "vm/engines/delay_i32.h"
#include "vm/engines/engine_i32_v1.h"

namespace fv1tests {

using TestVMI32 = TestVMImpl<fv1::engine::EngineI32, fv1::engine::DelayStorageI32, 1>;
using namespace fv1;

TEST_F(TestVMI32, first_run)
{
  auto &state = vm_.state();
  EXPECT_TRUE(state.first_run);

  vm_.Execute(in, out, 1);

  EXPECT_FALSE(state.first_run);
}

TEST_F(TestVMI32, Optimize)
{
  Compile("test_optimize.bin");
  size_t i = 0;

  EXPECT_EQ(OPCODE::RDFX, vm_.get_instruction(i++).get_opcode());
  EXPECT_EQ(OPCODE::LDAX, vm_.get_instruction(i).get_opcode());
  EXPECT_EQ(REGISTER::ADCL, vm_.get_instruction(i++).constants[0].loadi());

  EXPECT_EQ(OPCODE::MAXX, vm_.get_instruction(i++).get_opcode());
  EXPECT_EQ(OPCODE::ABSA, vm_.get_instruction(i++).get_opcode());

  EXPECT_EQ(OPCODE::AND, vm_.get_instruction(i).get_opcode());
  EXPECT_FALSE(vm_.get_instruction(i++).constants[0].zero());
  EXPECT_EQ(OPCODE::CLR, vm_.get_instruction(i).get_opcode());
  EXPECT_TRUE(vm_.get_instruction(i++).constants[0].zero());

  EXPECT_EQ(OPCODE::XOR, vm_.get_instruction(i++).get_opcode());
  EXPECT_EQ(OPCODE::NOT, vm_.get_instruction(i++).get_opcode());

  EXPECT_EQ(OPCODE::SKP, vm_.get_instruction(i++).get_opcode());
  EXPECT_EQ(OPCODE::JMP, vm_.get_instruction(i++).get_opcode());
  EXPECT_EQ(OPCODE::NOP, vm_.get_instruction(i++).get_opcode());
  EXPECT_EQ(OPCODE::NOP, vm_.get_instruction(i++).get_opcode());
}

TEST_F(TestVMI32, nop)
{
  Compile("test_nop.bin");
  vm_.Execute(in, out, 1);

  EXPECT_EQ(0, in[0].l);
  EXPECT_EQ(0, in[0].r);
}

TEST_F(TestVMI32, copy)
{
  Compile("test_copy.bin");

  in[0] = {SF23::MAX, SF23::MIN};
  vm_.Execute(in, out, 1);

  EXPECT_EQ(in[0], out[0]);

  auto &state = vm_.state();
  EXPECT_TRUE(state.acc_.zero());
}

TEST_F(TestVMI32, skp_jump)
{
  Compile("test_skp_jump.bin");
  in[0] = {SF23::MAX, 0};

  vm_.Execute(in, out, 1);
  EXPECT_EQ(in[0].l, out[0].l);
  EXPECT_EQ(0, out[0].r);
}

TEST_F(TestVMI32, skp_run)
{
  Compile("test_skp_run.bin");
  in[0] = {SF23::MAX, 0};

  vm_.Execute(in, out, 1);
  EXPECT_EQ(0, out[0].l);
  EXPECT_EQ(0, out[0].r);

  vm_.Execute(in, out, 1);
  EXPECT_EQ(in[0].l, out[0].l);
  EXPECT_EQ(0, out[0].r);
}

// NOTE Offset is .10 so [-1.0, 1.0)
TEST_F(TestVMI32, sof)
{
  Compile("test_sof.bin");

  vm_.Execute(in, out, 1);
  EXPECT_EQ((SF23::MAX + 1) / 2, out[0].l);
  EXPECT_EQ(SF23::MIN, out[0].r);
}

TEST_F(TestVMI32, mask)
{
  Compile("test_mask.bin");

  vm_.Execute(in, out, 1);

  auto &registers = vm_.state().registers_;
  EXPECT_EQ(registers[REG0].load(), 0);
  EXPECT_EQ(registers[REG1].load(), 0xf0f);
  EXPECT_EQ(registers[REG2].load(), SF23::MAX);
  EXPECT_EQ(registers[REG3].load(), SF23::MIN);
  EXPECT_EQ(registers[REG4].load(), -1);
  EXPECT_EQ((uint32_t)registers[REG4].loadi(), 0xffffffffU);
  EXPECT_EQ(registers[REG5].load(), 0x7fffff);
}

// It's possible to write arbitrary values into the ADDR_PTR register.
// According to fv1testing, the sign bit is ignored.
TEST_F(TestVMI32, RMPA)
{
  Compile("test_rmpa.bin");

  vm_.Execute(in, out, 1);

  auto &registers = vm_.state().registers_;
  EXPECT_EQ(registers[ADDR_PTR].load(), static_cast<int32_t>(0xff800200));
  EXPECT_EQ(0x123456, out[0].l);
}

TEST_F(TestVMI32, CHO_RDA_RMP)
{
  Compile("test_chorda_rmp.bin");

  in[0] = {SF23::MAX, SF23::MAX};
  for (int i = 0; i < 16; ++i) { vm_.Execute(in, out, 1); }
}

TEST_F(TestVMI32, RegisterFunctions)
{
  Compile("test_registers.bin");
  vm_.Execute(in, out, 1);

  auto &registers = vm_.state().registers_;
  EXPECT_EQ(registers[REG0].load(), 0x3fffff);
  EXPECT_EQ(registers[REG1].load(), 0x1fffff);
  EXPECT_EQ(registers[REG16].load(), 32 << 8);

  EXPECT_EQ(registers[REG2].load(), 0xfffff);

  // EXPECT_EQ(registers[ADDR_PTR].load(), 32 << 8);
  EXPECT_EQ(registers[REG4].load(), 0x12345);

  EXPECT_EQ(registers[ADDR_PTR].load(), (16384 + 32) << 8);
  EXPECT_EQ(registers[REG5].load(), 0x67890);

  // EXPECT_EQ(vm_.delay_memory().load_immediate(0), 0);
}

TEST_F(TestVMI32, RegisterFX)
{
  Compile("test_register_fx.bin");
  in[0].l = 0x7fffff;
  vm_.Execute(in, out, 1);

  auto &registers = vm_.state().registers_;
  // RDFX
  EXPECT_LE(registers[DACL].load().value,
            static_cast<int32_t>(0.1f * static_cast<float>(0x7fffff)));
  EXPECT_EQ(registers[DACL].load(), registers[REG0].load());

  // WRLX
  // DACR =  (PACC - ACC) x 0.2 + PACC
  //        (0 - 8388607) x 0.2 + 0 = -1677721
  EXPECT_EQ(registers[REG1].load(), 0x7fffff);
  EXPECT_EQ(registers[DACR].load(), -1677824);
}

}  // namespace fv1tests
