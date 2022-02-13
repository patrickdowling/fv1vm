# Experimental/Embedded FV-1 Virtual Machine

This is the public-facing and somewhat curated version of a private repository.

The main goal was to run [FV-1](http://www.spinsemi.com/knowledge_base/inst_syntax.html) programs/banks on a STM32F4. Why an F4? Because I have a bunch of F4 + codec hardware lying around.
The fixed-point engine (see below) does actually run a complex reverb using all 128 instructions on a STM32F405 at 168MHz using about 90% of the available processor time (sample rate 32KHz). The float version is WIP.

Other goals (which are sometimes orthogonal to the main one...)
- Experiment with C++ features
- Do things "differently" than I usually would (e.g. operator overloading for fixed-point math)

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

### Decoding
- Instead of writing this all out by hand, a hybrid template/macro implementation is used to parse the "instruction coding" fields in the SPIN asm user manual.
- These fields look something like `CCCCCCCCCCCCCCCC00000AAAAAA00100` where `C` and `A` are fields we'd like to extract.
- While the resulting operands are not strongly typed, they are at least annotated as to their function (e.g. S.23, S4.9, register index, address, mask, etc.)
- With some more effort one might be able to further automate this, but it seems "Good Enough" for now.
- Some disambiguation takes place during decoding for instructions that share an opcode, but different fields (`CHO RDA` or `CHO SOF`).
- Optimization (e.g. turning a `SKP` into a `JMP`) is handled later by the VM since it can also re-pack the operands, or generate artificial specific opcodes.

## Testing
- Yeah, unit tests are pretty thin still.
- To figure out specifics of the FV-1 behaviour that weren't described with enough depth, I took an approach similar to [ndf-zz/fv1testing](https://github.com/ndf-zz/fv1testing) and wrote programs to highlight specific operations.
- The results can then be simulated/checked by loading the same program on both hardware (hello Dervish!) and in the unit test.
- For other things, there's a tool to generate a .WAV file from a program (useful for LFO checks).

## Random Notes
- Sure, an F7 or H7 would be faster and has more memory. But where's the fun in that?
- A different approach would be to disassemble the FV-1 opcodes and generate C++ (or, just ARM assembler). That's "on the list" but the original goal was just to use existing banks.
