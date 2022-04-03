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

#ifndef FV1_VM_H_
#define FV1_VM_H_

#include <array>
#include <cstdint>
#include <type_traits>

#include "fv1/fv1_defs.h"
#include "fv1/fv1_delay_memory.h"
#include "fv1/fv1_opcodes.h"
#include "fv1/fv1_registers.h"
#include "ramp_lfo.h"
#include "sin_lfo.h"

namespace fv1 {

class ProgramStream;
struct DecodedInstruction;

// The vague idea is the implement the VM without knowing too many details about the underlying
// math, and theoretically being able to replace it e.g. with "float math". The way it ended up is
// that there two very inter-dependent things now (VM, Engine) so it'd probably be better to move
// the main part of Execute into the engine, and perhaps the state also. Then the VM is basically
// the translator/compiler from DecodedInstruction to the "byte code".
//
// TODO We can save the compiled instructions since they are self-contained. We'd have to add some
// additional information like version and size though, but then the compiled results could be
// cached.
//
// A further extension of that would be add the state/delay into a persistable package, allowing for
// a single VM to process multiple programs?

template <typename Engine, typename DelayStorage>
class VM {
public:
  static_assert(
      std::is_same<typename Engine::float_value, typename DelayStorage::value_type>::value);

  using DelayMemoryBuffer = typename DelayMemory<DelayStorage>::buffer_type;
  using AudioFrame = AudioFrameT<typename Engine::float_type>;
  using Parameters = ParametersT<typename Engine::float_type>;

  // Delay memory is maintained externally
  explicit VM(DelayMemoryBuffer &delay_memory_buffer);

  // Compile program
  void Compile(ProgramStream &program);

  // control values used for all frames
  void SetParameters(const Parameters &params)
  {
    state_.registers_[POT0].store(params.pots[0]);
    state_.registers_[POT1].store(params.pots[1]);
    state_.registers_[POT2].store(params.pots[2]);
  }

  // Execute the compiled program on each frame in a block
  void Execute(const AudioFrame *in, AudioFrame *out, size_t num_frames);

  // --
  // Technically these are internal details but it makes it easier for tests

  // Compiled instructions are effectively the VM's byte code.
  // Fixed size makes jumps easier.
  struct CompiledInstruction {
    OPCODE opcode = OPCODE::NOP;
    // There should be three bytes of padding in here in case we want to store additional info
    std::array<typename Engine::Constant, kMaxOperands> constants;

    inline OPCODE get_opcode() const { return opcode; }
    inline void set_opcode(OPCODE o) { opcode = o; }
  };
  static_assert(sizeof(CompiledInstruction) == 16);

  struct State {
    bool first_run = true;
    typename Engine::Register acc_;
    typename Engine::Register pacc_;
    std::array<typename Engine::Register, kNumRegisters> registers_;

    void Reset()
    {
      first_run = true;
      acc_.clr();
      pacc_.clr();
      for (auto &r : registers_) r.clr();
    }
  };

  const State &state() const { return state_; }
  const CompiledInstruction &get_instruction(size_t i) const { return instructions_[i]; }
  const DelayMemory<DelayStorage> &delay_memory() const { return delay_memory_; }

private:
  using IndexConstant = IndexConstantT<typename Engine::Constant>;
  using IntegerConstant = IntegerConstantT<typename Engine::Constant>;
  using FloatConstant = FloatConstantT<typename Engine::Constant>;

  using RampLfo = RampLfoImpl<Engine>;
  using SinLfo = SinLfoImpl<Engine>;

  std::array<CompiledInstruction, kMaxInstructionCount> instructions_;

  State state_;
  DelayMemory<DelayStorage> delay_memory_;
  std::array<RampLfo, 2> ramp_lfo_;
  std::array<SinLfo, 2> sin_lfo_;

  // "Fake" index used in VM
  enum CHO_SEL_IDX : int32_t { SIN0_SIN = 0, SIN0_COS, SIN1_SIN, SIN1_COS, RMP0_VAL, RMP1_VAL };

  SF23 read_lfo(CHO_SEL_IDX idx) const
  {
    switch (idx) {
      case CHO_SEL_IDX::SIN0_SIN: return sin_lfo_[0].sin(); break;
      case CHO_SEL_IDX::SIN0_COS: return sin_lfo_[0].cos(); break;
      case CHO_SEL_IDX::SIN1_SIN: return sin_lfo_[1].sin(); break;
      case CHO_SEL_IDX::SIN1_COS: return sin_lfo_[1].cos(); break;
      case CHO_SEL_IDX::RMP0_VAL: return ramp_lfo_[0].value(); break;
      case CHO_SEL_IDX::RMP1_VAL: return ramp_lfo_[1].value(); break;
    }
    return SF23{0};
  }

  static CompiledInstruction CompileInstruction(const DecodedInstruction &instruction);
  void Optimize();
  void Tick()
  {
    delay_memory_.Tick();
    for (auto &rmp : ramp_lfo_) rmp.Tick();
    for (auto &sin : sin_lfo_) sin.Tick();
  }
};
}  // namespace fv1

// clang-format off
#include "vm_impl.h"
#include "vm_execute_v1.h"
// clang-format on

#endif  // FV1_VM_H_
