# Experimental/Embedded FV-1 Virtual Machine

This is the public-facing and somewhat curated version of a private repository.

The main goal was to run [FV-1](http://www.spinsemi.com/knowledge_base/inst_syntax.html) programs/banks on a STM32F4. Why an F4? Because I have a bunch of F4 + codec hardware lying around.
The fixed-point engine (see below) does actually run a complex reverb using all 128 instructions on a STM32F405 at 168MHz using about 90% of the available processor time (sample rate 32KHz). The float version is WIP.

Other goals (which are sometimes orthogonal to the main one...)
- Experiment with C++ features
- Do things "differently" than I usually would (e.g. operator overloading for fixed-point math)

![CI Badge](https://github.com/patrickdowling/fv1vm/actions/workflows/make.yml/badge.svg)

## Structure
There are three main layers:
- Decoding binary instructions into an internal representation. No, it doesn't read .asm files, only banks or compiled programs.
- A VM that can compile the decoded instructions into a (somewhat optimized) "byte code" and run the program per sample. This includes things like
  - Registers
  - The delay memory (\*)
  - RMP and SIN LFOs
- The VM can use an `Engine` implementation to actually execute the operations. There are currently two implementations:
  1. A purely fixed-point/`int32_t` version that should be fairly close to the S.23 used in the FV-1.
  2. A mixed fixed-point/`float32` version that ideally will be faster (\*\*). Mixed because some operations (`AND`, `OR`, `NOT`) still operate on 24-bit values.

(\*) Since the F4 used only has 128K or RAM (plus CCM) and we need 32K memory locations, this uses `__fp16` which is easy to convert to and from.

(\*\*) There's some caveats. All the math operations are single instructions since there's no shifts required, but clipping the result as a float is ugly (two compares instead of a `SSAT`)
It's perhaps not strictly necessary and can be mostly avoided (e.g. until register assignment) but removing it altogether seems questionable.

### Binary Instruction Decoding
- Instead of writing this all out by hand, a hybrid template/macro implementation is used to parse the "instruction coding" fields in the SPIN asm user manual.
- These fields look something like `CCCCCCCCCCCCCCCC00000AAAAAA00100` where `C` and `A` are fields we'd like to extract.
- While the resulting operands are not strongly typed, they are at least annotated as to their function (e.g. S.23, S4.9, register index, address, mask, etc.)
- With some more effort one might be able to further automate this, but it seems "Good Enough" for now.
- Some disambiguation takes place during decoding for instructions that share an opcode, but different fields (`CHO RDA` or `CHO SOF`).
- Optimization (e.g. turning a `SKP` into a `JMP`) is handled later by the VM since it can also re-pack the operands, or generate artificial specific opcodes.
- The basic idea is also to try and fail at _compile time_ through static assertions, rather than runtime.

While the code looks fairly complex and there's a lot of boilerplate, the combination of templates + `constexpr` produces the desired outcome and everything "collapses" at runtime.
E.g. `Instruction::DecodeOperand` (which builds the code to extract a field from a string, the char id, and a type) boils down to basically `ubfx` (there's probably still some room for improvement by makeing the shifts & ands more idiomatic).

## Testing
- Yeah, unit tests are pretty thin still.
- To figure out specifics of the FV-1 behaviour that weren't described with enough depth, I took an approach similar to [ndf-zz/fv1testing](https://github.com/ndf-zz/fv1testing) and wrote programs to highlight specific operations.
- The results can then be simulated/checked by loading the same program on both hardware (hello Dervish!) and in the unit test.
- For other things, there's a tool to generate a .WAV file from a program (useful for LFO checks).

## VM
- The version here is just the tip of the iceberg.
- Things like JIT or even emitting ARM assembly snippets for the individual opcodes are "on the list".
- Instead of the VM bytecode operating on the FV1 instructions, we could break each individual instruction down further into the individual ops (load, store, mac, etc.).
- This would be fun but seems like that would a) add more overhead -- but b) also yield more opportunity to optimize the bytecode (e.g. to merge LFO common access patterns).

## Random Notes
- Sure, an F7 or H7 would be faster and has more memory. But where's the fun in that?
- A different approach would be to disassemble the FV-1 opcodes and generate C++ (or, just ARM assembler). That's "on the list" but the original goal was just to use existing banks.
- The instruction decoder table can, with some effort, but made constexpr so the `Register` functions "go away".
