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
//
#ifndef FV1_VM_H_
#error "Don't include or compile this file directly"
#endif

#include "fv1/debug/fv1_debug.h"
#include "fv1/fv1_asm_decode.h"
#include "misc/program_stream.h"

#define GET_CONSTANT(type, name, index) \
  const type name { instruction.constants[index] }
#define IDX(name) IndexConstant, name
#define INT(name) IntegerConstant, name
#define FLOAT(name) FloatConstant, name
#define GET_INT_CONSTANT(name, index) GET_CONSTANT(IntegerConstant, name, index)

namespace fv1 {

template <typename Engine, typename DelayStorage>
/*static*/ typename VM<Engine, DelayStorage>::CompiledInstruction
VM<Engine, DelayStorage>::CompileInstruction(const DecodedInstruction &instruction)
{
  std::array<typename Engine::Constant, kMaxOperands> constants;

  for (size_t i = 0; i < instruction.num_operands; ++i) {
    auto operand = instruction.operands[i];
    if (operand.is_fixed_point())
      constants[i].store(SF23{operand.value});
    else if (operand.is_mask())  // This should be unneeded, but...
      constants[i].store(SF23::MASK & operand.value);
    else if (!operand.is_none())
      constants[i].store(operand.value);
  }
  return {instruction.opcode(), constants};
}

template <typename Engine, typename DelayStorage>
VM<Engine, DelayStorage>::VM(DelayMemoryBuffer &memory_buffer)
    : delay_memory_{memory_buffer},
      ramp_lfo_{
          {{&state_.registers_[REGISTER::RMP0_RATE], &state_.registers_[REGISTER::RMP0_RANGE]},
           {&state_.registers_[REGISTER::RMP1_RATE], &state_.registers_[REGISTER::RMP1_RANGE]}}},
      sin_lfo_{
          {{&state_.registers_[REGISTER::SIN0_RATE], &state_.registers_[REGISTER::SIN0_RANGE]},
           {&state_.registers_[REGISTER::SIN1_RATE], &state_.registers_[REGISTER::SIN1_RANGE]}}}
{}

template <typename Engine, typename DelayStorage>
void VM<Engine, DelayStorage>::Compile(ProgramStream &program)
{
  state_.Reset();
  delay_memory_.Reset();
  for (auto &rmp : ramp_lfo_) rmp.Jam();
  for (auto &sin : sin_lfo_) sin.Jam();

  size_t instruction_count = 0;
  while (program.available()) {
    auto di = InstructionDecoder::Decode(program.Next());
    instructions_[instruction_count] = CompileInstruction(di);
    ++instruction_count;
  }

  Optimize();
}

// NOTES
// - Patterns of CHO (i.e. interpolation) that are two reads from the same LFO with 1-C and C
template <typename Engine, typename DelayStorage>
void VM<Engine, DelayStorage>::Optimize()
{
  for (auto &instruction : instructions_) {
    auto opcode = instruction.get_opcode();
    switch (opcode) {
      // RDFX: If C is zero, just load accumulator => LDAX
      case OPCODE::RDFX:
        if (instruction.constants[1].zero()) instruction.set_opcode(OPCODE::LDAX);
        break;

        // MAXX: If C 0, get absolute value of accumulator
      case OPCODE::MAXX:
        if (instruction.constants[1].zero()) instruction.set_opcode(OPCODE::ABSA);
        break;

      // Logical operations: double-check mask
      // AND: if no mask => CLR
      // XOR: if all bits => NOT
      case OPCODE::AND:
      case OPCODE::OR:
      case OPCODE::XOR:
        if (OPCODE::AND == opcode && instruction.constants[0].zero())
          instruction.set_opcode(OPCODE::CLR);
        else if (OPCODE::XOR == opcode && 0xffffff == instruction.constants[0].loadi())
          instruction.set_opcode(OPCODE::NOT);
        break;

      // SKP with no offset => NOP
      // SKP with no flags => JMP
      case OPCODE::SKP:
        if (instruction.constants[1].zero()) {
          instruction.set_opcode(OPCODE::NOP);
        } else if (instruction.constants[0].zero()) {
          instruction.set_opcode(OPCODE::JMP);
        }
        break;

      // WLDS: Pre-shift values (n, f, a)
      case OPCODE::WLDS: {
        GET_INT_CONSTANT(f, 1);  // 0-511
        GET_INT_CONSTANT(a, 2);  // 0-32767
        instruction.constants[1].store(SF23{f << SinLfo::kRateShift});
        instruction.constants[2].store(SF23{f << SinLfo::kRangeShift});
      } break;

      // WLDR: Pre-shift values (n, f, a)
      case OPCODE::WLDR: {
        // [1] = rate = I16
        // [2] = range = 0x0, 0x1, 0x2, 0x3, so we'll convert to register
        GET_INT_CONSTANT(a, 2);
        instruction.constants[2].store(SF23{a << RampLfo::kRangeShift});
      } break;

      // CHO_RDA
      // CHO_SOF: Dispatch to rmp/sin specific things, n becomes 0/1
      case OPCODE::CHO_RDA:
      case OPCODE::CHO_SOF: {
        GET_INT_CONSTANT(n, 0);
        // auto flags = instruction.constants[1].load();
        int32_t idx = 0;
        auto new_opcode = opcode;
        switch (n.template enum_cast<CHO_SEL, CHO_SEL_MASK>()) {
          case CHO_SEL::SIN0:
          case CHO_SEL::SIN1:
            idx = n - CHO_SEL::SIN0;
            new_opcode = OPCODE::CHO_RDA == opcode ? OPCODE::CHO_RDA_SIN : OPCODE::CHO_SOF_SIN;
            break;
          case CHO_SEL::RMP0:
          case CHO_SEL::RMP1:
            idx = n - CHO_SEL::RMP0;
            new_opcode = OPCODE::CHO_RDA == opcode ? OPCODE::CHO_RDA_RMP : OPCODE::CHO_SOF_RMP;
            break;
        }
        instruction.set_opcode(new_opcode);
        instruction.constants[0].store(idx);
      } break;

      // CHO_RDAL: Simplify LFO lookup
      case OPCODE::CHO_RDAL: {
        GET_INT_CONSTANT(n, 0);
        GET_INT_CONSTANT(flags, 1);
        int32_t idx = 0;
        bool cos = CHO_FLAGS::COS & flags;
        switch (n.template enum_cast<CHO_SEL, CHO_SEL_MASK>()) {
          case CHO_SEL::SIN0: idx = cos ? CHO_SEL_IDX::SIN0_COS : CHO_SEL_IDX::SIN0_SIN; break;
          case CHO_SEL::SIN1: idx = cos ? CHO_SEL_IDX::SIN1_COS : CHO_SEL_IDX::SIN1_SIN; break;
          case CHO_SEL::RMP0: idx = CHO_SEL_IDX::RMP0_VAL; break;
          case CHO_SEL::RMP1: idx = CHO_SEL_IDX::RMP1_VAL; break;
        }
        instruction.constants[0].store(idx);
      } break;

      default: break;
    }
  }
}

}  // namespace fv1
