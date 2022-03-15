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

#include "fv1/fv1_instruction.h"
#include "misc/program_stream.h"

// Some ideas
// - We can store a pointer to the register in the CompiledInstruction
// - We can pack a function pointer in there as well as skip the switch dispatch
// - ...but then we have a guaranteed function call?
//
// The general idea is that the operands we get can be converted to a dedicated format for the VM.
// The types are annotated (but but strictly enforced). For the (simple) i32 case almost everything
// is converted to SF23 so working with registers is "easy". It _should_ in theory also be possible
// to use a float-based math engine but there are some odd boundary cases (like masks).
//
// Float math is simpler (e.g. the mult is a single op) but clipping fixed-point boils down to a
// ssat instruction instead of two compares. OTOH the whole LFO implementation seems to be based
// around ints as well, so the answer seems to be "it depends".
//
// TODO CHO flags, REG. Since the simulated LFO/SIN are synchrounous this seems unneeded?
// Bit1: REG (000010) function. Using this will register the current LFO outputs for subsequent
// operations; to be used only on the first access to an LFO (LFO is continuously updated in the
// background)
// TODO We could also do a per-instruction increment for finer interpolation
// TODO There might still be benefit of an explicit MAC for float => VMLA

namespace fv1 {

//
#define OPCODE_DISPATCH_TODO(x) OPCODE_DISPATCH_NOP(x)
//
#define OPCODE_DISPATCH_NOP(x) \
  case OPCODE::x: break
//
#define OPCODE_DISPATCH_0(x) \
  case OPCODE::x: {          \
    do {                     \
    } while (0)
//
#define OPCODE_DISPATCH_1(x, c0) \
  case OPCODE::x: {              \
    GET_CONSTANT(c0, 0)
//
#define OPCODE_DISPATCH_2(x, c0, c1) \
  case OPCODE::x: {                  \
    GET_CONSTANT(c0, 0);             \
    GET_CONSTANT(c1, 1)
//
#define OPCODE_DISPATCH_3(x, c0, c1, c2) \
  case OPCODE::x: {                      \
    GET_CONSTANT(c0, 0);                 \
    GET_CONSTANT(c1, 1);                 \
    GET_CONSTANT(c2, 2)
//
#define OPCODE_END() \
  }                  \
  break

template <typename Engine, typename DelayStorage>
void VM<Engine, DelayStorage>::Execute(const AudioFrame *in, AudioFrame *out, size_t num_frames)
{
  typename Engine::Register acc = state_.acc_;
  typename Engine::Register pacc = state_.pacc_;
  typename Engine::Register prev_acc = acc;
  auto registers = state_.registers_.data();
  const auto instructions = instructions_.data();

  for (; num_frames; --num_frames, ++in, ++out) {
    state_.registers_[ADCL].store(in->l);
    state_.registers_[ADCR].store(in->r);

    int32_t ic = 0;
    while (ic < kMaxInstructionCount) {
      auto &instruction = instructions[ic];
      switch (instruction.get_opcode()) {
        // Order of opcodes is based on hex value. It might also make sense to group by
        // functionality

        OPCODE_DISPATCH_2(RDA, INT(addr), FLOAT(c));
        acc.store(delay_memory_.Load(addr) * c + acc.load());
        OPCODE_END();

        OPCODE_DISPATCH_1(RMPA, FLOAT(c));
        auto ptr = registers[ADDR_PTR].load_addr();
        acc.store(delay_memory_.Load(ptr) * c + acc.load());
        OPCODE_END();

        OPCODE_DISPATCH_2(WRA, INT(addr), FLOAT(c));
        delay_memory_.Store(addr, acc);
        acc.store(acc.load() * c);
        OPCODE_END();

        OPCODE_DISPATCH_2(WRAP, INT(addr), FLOAT(c));
        delay_memory_.Store(addr, acc);
        acc.store(acc.load() * c + delay_memory_.last_read());
        OPCODE_END();

        OPCODE_DISPATCH_2(RDAX, INT(addr), FLOAT(c));
        acc.store(registers[addr].load() * c + acc.load());
        OPCODE_END();

        OPCODE_DISPATCH_2(RDFX, INT(addr), FLOAT(c));
        auto r = registers[addr].load();
        acc.store((acc.load() - r) * c + r);
        OPCODE_END();

        OPCODE_DISPATCH_2(WRAX, INT(addr), FLOAT(c));
        registers[addr].store(acc);
        acc.store(acc.load() * c);
        OPCODE_END();

        OPCODE_DISPATCH_2(WRHX, INT(addr), FLOAT(c));
        registers[addr].store(acc);
        acc.store(acc.load() * c + pacc.load());
        OPCODE_END();

        OPCODE_DISPATCH_2(WRLX, INT(addr), FLOAT(c));
        registers[addr].store(acc);
        acc.store((pacc.load() - acc.load()) * c + pacc.load());
        OPCODE_END();

        OPCODE_DISPATCH_2(MAXX, INT(addr), FLOAT(c));
        auto abs_rxc = Engine::ABS(registers[addr].load() * c);
        auto abs_acc = Engine::ABS(acc.load());
        acc.store(abs_rxc > abs_acc ? abs_rxc : abs_acc);
        OPCODE_END();

        OPCODE_DISPATCH_1(MULX, INT(addr));
        acc.store(acc.load() * registers[addr].load());
        OPCODE_END();

        OPCODE_DISPATCH_TODO(LOG);
        OPCODE_DISPATCH_TODO(EXP);

        OPCODE_DISPATCH_2(SOF, FLOAT(c), FLOAT(d));
        acc.store(acc.load() * c + d);
        OPCODE_END();

        // Logical ops always use SF23
        OPCODE_DISPATCH_1(AND, INT(mask));
        acc.storei(core::AND<SF23>(acc.loadi(), mask));
        OPCODE_END();

        OPCODE_DISPATCH_1(OR, INT(mask));
        acc.storei(core::OR<SF23>(acc.loadi(), mask));
        OPCODE_END();

        OPCODE_DISPATCH_1(XOR, INT(mask));
        acc.storei(core::XOR<SF23>(acc.loadi(), mask));
        OPCODE_END();

        OPCODE_DISPATCH_2(SKP, INT(cmask), INT(n));  // NOTE Optimized to JMP if cmask == 0;
        bool skip = true;
        // All conditions must be met
        if (SKP_FLAGS::NEG & cmask) skip = skip && acc.neg();
        if (SKP_FLAGS::GEZ & cmask) skip = skip && acc.gez();
        if (SKP_FLAGS::ZRO & cmask) skip = skip && acc.zero();
        if (SKP_FLAGS::ZRC & cmask) skip = skip && acc.gez() != pacc.gez();
        if (SKP_FLAGS::RUN & cmask) skip = skip && !state_.first_run;
        if (skip) ic += n;
        OPCODE_END();

        // NOTE Pre-shifted values from VM::Optimize
        OPCODE_DISPATCH_3(WLDS, IDX(n), FLOAT(f), FLOAT(a));
        if (n) {
          registers[REGISTER::SIN1_RATE].store(f);
          registers[REGISTER::SIN1_RANGE].store(a);
          sin_lfo_[1].Jam();
        } else {
          registers[REGISTER::SIN0_RATE].store(f);
          registers[REGISTER::SIN0_RANGE].store(a);
          sin_lfo_[0].Jam();
        }
        OPCODE_END();

        OPCODE_DISPATCH_1(JAM, IDX(n));
        ramp_lfo_[n].Jam();
        OPCODE_END();

        OPCODE_DISPATCH_0(CLR);
        acc.clr();
        OPCODE_END();

        OPCODE_DISPATCH_0(NOT);
        acc.storei(core::NOT<SF23>(acc.loadi()));
        OPCODE_END();

        OPCODE_DISPATCH_0(ABSA);
        acc.store(Engine::ABS(acc.load()));
        OPCODE_END();

        OPCODE_DISPATCH_1(LDAX, INT(addr));
        acc.store(registers[addr]);
        OPCODE_END();

        // NOTE Pre-shifted values from VM::Optimize
        OPCODE_DISPATCH_3(WLDR, IDX(n), INT(f), INT(a));
        if (n) {
          registers[REGISTER::RMP1_RATE].store(f);
          registers[REGISTER::RMP1_RANGE].store(a);
          ramp_lfo_[1].Jam();
        } else {
          registers[REGISTER::RMP0_RATE].store(f);
          registers[REGISTER::RMP0_RANGE].store(a);
          ramp_lfo_[0].Jam();
        }
        OPCODE_END();

        // ********************************************************************************
        // CHO variants
        // - These are still very similar so it might make sense to unify further
        // - OTOH we can only go so far without adding something like a function pointer to the
        // instruction struct, or other additional dispatch.
        //
        // TODO Check handling of COMPA (does the complement affect the coefficient?)

        OPCODE_DISPATCH_1(CHO_RDAL, INT(n) /*, flags*/);
        // NOTE n is artificial from VM::Optimize so flags not needed
        acc.store(read_lfo(n.template enum_cast<CHO_SEL_IDX>()));
        OPCODE_END();

        // CHO RDA: ACC <- ACC + coeff (LFO) * delay[ADDRESS + offset (LFO)]
        OPCODE_DISPATCH_3(CHO_RDA_RMP, IDX(n), INT(flags), INT(addr));
        const auto lfo_value = ramp_lfo_[n].Read(flags.template enum_cast<CHO_FLAGS>());
        acc.store(delay_memory_.Load(addr + lfo_value.offset) * lfo_value.coefficient + acc.load());
        OPCODE_END();

        OPCODE_DISPATCH_3(CHO_RDA_SIN, IDX(n), INT(flags), INT(addr));
        const auto lfo_value = sin_lfo_[n].Read(flags.template enum_cast<CHO_FLAGS>());
        acc.store(delay_memory_.Load(addr + lfo_value.offset) * lfo_value.coefficient + acc.load());
        OPCODE_END();

        // CHO SOF: ACC <- coeff (LFO) * ACC + OFFSET
        OPCODE_DISPATCH_3(CHO_SOF_RMP, IDX(n), INT(flags), FLOAT(d));
        const auto lfo_value = ramp_lfo_[n].Read(flags.template enum_cast<CHO_FLAGS>());
        acc.store(acc.load() * lfo_value.coefficient + d);
        OPCODE_END();

        OPCODE_DISPATCH_3(CHO_SOF_SIN, IDX(n), INT(flags), FLOAT(d));
        const auto lfo_value = sin_lfo_[n].Read(flags.template enum_cast<CHO_FLAGS>());
        acc.store(acc.load() * lfo_value.coefficient + d);
        OPCODE_END();

        // ********************************************************************************

        OPCODE_DISPATCH_2(JMP, INT(flags), INT(n));
        // ignoring flags
        (void)flags;
        ic += n;
        OPCODE_END();

        OPCODE_DISPATCH_NOP(CHO_RDA);  // optimized away
        OPCODE_DISPATCH_NOP(CHO_SOF);  // optimized away
        OPCODE_DISPATCH_NOP(NOP);
        OPCODE_DISPATCH_NOP(UNKNOWN);
        OPCODE_DISPATCH_NOP(REAL_OPCODES_LAST);
      }
      // per-opcode updates
      // We want pacc to be one state delayed, i.e. from before the last execution.
      // Otherwise instructions like WRLX that use (PACC - ACC) dont' make much sense.
      pacc = prev_acc;
      prev_acc = acc;
      ++ic;
    }
    Tick();
    state_.first_run = false;
    state_.registers_[DACL].read(out->l);
    state_.registers_[DACR].read(out->r);
  }

  state_.pacc_ = pacc;
  state_.acc_ = acc;
}

}  // namespace fv1
