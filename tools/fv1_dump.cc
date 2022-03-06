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

#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "fv1/debug/fv1_debug.h"
#include "fv1/fv1_asm_decode.h"
#include "fv1_tools.h"
#include "misc/program_stream.h"

#define VERBOSE(...) \
  if (options.verbose) INFO(__VA_ARGS__)

static struct option long_opts[] = {
    {"file", required_argument, nullptr, 'f'}, {"help", no_argument, nullptr, 'h'},
    {"nodump", no_argument, nullptr, 'n'},     {"program", required_argument, nullptr, 'p'},
    {"verbose", no_argument, nullptr, 'v'},    {nullptr, 0, nullptr, 0},
};

static const char *short_opts = "f:hnp:v";

static void Usage()
{
  printf(
      "fv1_dump options <filename or stdin>: Disasemble binary file and print "
      "instruction/opcodes\n");
  printf(" --file\t-f\tName of bank or binary file to read. If not specified, read from stdin\n");
  printf(" --help\t-h\tShow this message\n");
  printf(" --nodump\t-n\tOnly print program info if available\n");
  printf(" --program\t-p\tIf source is a banke file, program in bank to decode (0-7)\n");
  printf(" --verbose\t-v\tMore output\n");
}

static struct {
  std::string file = "";
  bool nodump = false;
  int program = 0;
  bool verbose = false;
} options;

bool ParseCommandLine(int argc, char **argv)
{
  int ch = 0;
  do {
    ch = getopt_long(argc, argv, short_opts, long_opts, NULL);
    switch (ch) {
      case 'f': options.file = optarg; break;
      case 'h': return false;
      case 'n': options.nodump = true; break;
      case 'p': options.program = atoi(optarg); break;
      case 'v': options.verbose = true; break;
      case '?': return false;
      case 0:
      case -1:
      default: break;
    }
  } while (-1 != ch);

  if (options.program < 0 || options.program > 7) return false;

  return true;
}

static void disassemble(const char *binary)
{
  VERBOSE("** DISASSEMBLY");
  using namespace fv1;

  uint32_t last = 0xffffffff;
  int count = 0;
  int ic = 0;

  BufferStream<BSWAP_ENABLE> program{binary};
  while (program.available()) {
    auto instruction = program.Next();
    if (instruction == last) {
      ++count;
    } else {
      auto di = InstructionDecoder::Decode(instruction);
      debug::print_decoded_instruction(ic, di, options.verbose);
      if (count > 1) printf("... x%d", count);
      last = instruction;
      count = 1;
    }
    ++ic;
  }
  if (count > 1) printf("... x%d", count);
  printf("\n");
}

static fv1tools::BinaryFile binary_file;

int main(int argc, char **argv)
{
  if (!ParseCommandLine(argc, argv)) {
    Usage();
    return EXIT_FAILURE;
  }

  if (!binary_file.Read(options.file))
    ERR("** Failed to read input file '%s'", strerror(errno));
  else
    INFO("** Read %zu bytes from '%s'", binary_file.length(), options.file.c_str());

  using namespace fv1;
  if (!binary_file.valid_length()) {
    ERR("%zu bytes, what is it?", binary_file.length());
    return EXIT_FAILURE;
  }
  if (options.verbose) fv1tools::print_bank_type(binary_file.length());

  // If available, the program description is tacked on the end of the buffer
  fv1tools::print_program_info(binary_file.get_bank_info(), options.program);

  if (!options.nodump) {
    auto program = binary_file.program(options.program);
    if (!program) {
      ERR("Invalid program index %d", options.program);
      return EXIT_FAILURE;
    }

    disassemble(program);
  }

  return EXIT_SUCCESS;
}
