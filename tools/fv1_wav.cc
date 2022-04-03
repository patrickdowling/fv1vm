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

#include <cstdlib>
#include <string>
#include <vector>

#include "fv1_tools.h"
#include "misc/program_stream.h"
#include "vm/engines/delay_i32.h"
#include "vm/engines/engine_i32_v1.h"
#include "vm/vm.h"
#include "wav.h"

static constexpr uint32_t kSampleRate = 32000U;
static constexpr uint16_t kNumChannels = 2;
static constexpr size_t kBlockSize = 32;

#define VERBOSE(...) \
  if (options.verbose) INFO(__VA_ARGS__)

static struct option long_opts[] = {
    {"bits_per_sample", required_argument, nullptr, 'b'},
    {"blocksize", required_argument, nullptr, 'z'},
    {"file", required_argument, nullptr, 'f'},
    {"ofile", required_argument, nullptr, 'o'},
    {"program", required_argument, nullptr, 'p'},
    {"program_info", no_argument, nullptr, 'i'},
    {"sample_count", required_argument, nullptr, 's'},
    {"verbose", no_argument, nullptr, 'v'},
    {nullptr, 0, nullptr, 0},
};

static const char *short_opts = "b:f:io:p:s:vz:";

static struct {
  std::string file = "";
  std::string ofile = "";
  int program = 0;
  size_t sample_count = 0;
  uint16_t bits_per_sample = 16;
  size_t blocksize = 32;

  bool program_info = false;
  bool verbose = false;

  float pots[fv1::kNumPots] = {0.f};

} options;

void Usage()
{
  INFO(" --bits_per_sample\t-b\t16*|24");
  INFO(" --blocksize\t-z\tBlocksize (%zu)", kBlockSize);
  INFO(" --file\t-f\tProgram/bank input file");
  INFO(" --ofile\t-o\tOutput WAV file");
  INFO(" --program\t-p\tNumber of program to use if bank file(0-7)");
  INFO(" --program_info\t-i\tPrint program info");
  INFO(" --sample_count\t-s\tNumber of samples to compute");
  INFO(" --verbose\t-v\tExtra output");
}

bool ParseCommandLine(int argc, char **argv)
{
  int ch = 0;
  do {
    ch = getopt_long(argc, argv, short_opts, long_opts, NULL);
    switch (ch) {
      case 'b': sscanf(optarg, "%hu", &options.bits_per_sample); break;
      case 'f': options.file = optarg; break;
      case 'i': options.program_info = true; break;
      case 'o': options.ofile = optarg; break;
      case 'p': options.program = atoi(optarg); break;
      case 's': sscanf(optarg, "%zu", &options.sample_count); break;
      case 'z': sscanf(optarg, "%zu", &options.blocksize); break;
      case 'v': options.verbose = true; break;
      case '?': return false;
      case 0:
      case -1:
      default: break;
    }
  } while (-1 != ch);

  if (options.program < 0 || options.program > 7) return false;
  if (!options.sample_count) return false;
  if (options.ofile.empty()) return false;
  if (options.bits_per_sample != 16 && options.bits_per_sample != 24) return false;
  if (!options.blocksize) return false;

  while (optind < argc) {
    // Le grand hack
    int pot = -1;
    float value = 0.f;
    if (2 == sscanf(argv[optind++], "pot%d=%f", &pot, &value)) {
      if (pot >= 0 && pot < fv1::kNumPots) { options.pots[pot] = std::clamp(value, 0.f, 1.f); }
    }
  }
  return true;
}

using VM = fv1::VM<fv1::engine::EngineI32, fv1::engine::DelayStorageI32>;

static fv1tools::BinaryFile binary_file;
static VM::DelayMemoryBuffer delay_memory_buffer;
static VM vm{delay_memory_buffer};

int main(int argc, char **argv)
{
  if (!ParseCommandLine(argc, argv)) {
    Usage();
    return EXIT_FAILURE;
  }

  if (!binary_file.Read(options.file)) {
    ERR("** Failed to read input file '%s': %s", options.file.c_str(), strerror(errno));
    return EXIT_FAILURE;
  } else {
    VERBOSE("** Read %zu bytes from '%s'", binary_file.length(), options.file.c_str());
  }

  if (!binary_file.valid_length()) {
    ERR("%zu bytes, what is it?", binary_file.length());
    return EXIT_FAILURE;
  }
  if (options.program_info)
    fv1tools::print_program_info(binary_file.get_bank_info(), options.program);

  auto p = binary_file.program(options.program);
  if (!p) {
    ERR("Invalid program index %d", options.program);
    return EXIT_FAILURE;
  }

  fv1::BufferStream<fv1::BSWAP_ENABLE> program{p};
  vm.Compile(program);
  VERBOSE("** Compiled program");

  int ofile = open(options.ofile.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                   S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
  if (ofile < 0) {
    ERR("** Failed to open output file '%s'", strerror(errno));
    return EXIT_FAILURE;
  }

  uint16_t bytes_per_sample = options.bits_per_sample / 8;

  wav::WAVHeader wav_header;
  wav_header.riff_header.size =
      static_cast<uint32_t>(sizeof(wav_header) - sizeof(wav::ChunkHeader) +
                            options.sample_count * kNumChannels * bytes_per_sample);
  wav_header.fmt.format_tag = 1;               // PCM
  wav_header.fmt.num_channels = kNumChannels;  // stereo
  wav_header.fmt.sample_rate = kSampleRate;
  wav_header.fmt.byte_rate = kSampleRate * kNumChannels * bytes_per_sample;
  wav_header.fmt.block_align = static_cast<uint16_t>(kNumChannels * bytes_per_sample);
  wav_header.fmt.bits_per_sample = static_cast<uint16_t>(options.bits_per_sample);
  wav_header.data.size = 2U * static_cast<uint32_t>(options.sample_count) * bytes_per_sample;

  auto s = write(ofile, &wav_header, sizeof(wav_header));
  (void)s;
  wav::SampleWriter sample_writer{ofile, options.bits_per_sample};

  VERBOSE("** Running...");

  std::vector<VM::AudioFrame> in;
  std::vector<VM::AudioFrame> out;
  VM::Parameters params;

  in.resize(options.blocksize);
  out.resize(options.blocksize);
  for (int i = 0; i < fv1::kNumPots; ++i) {
    params.pots[i] = core::FixedFromFloat<fv1::SF23>(options.pots[i]).value;
    VERBOSE("** POT%d=%.2f (%06x)", i, (double)options.pots[i], params.pots[i]);
  }

  auto sample_count = options.sample_count;
  while (sample_count) {
    auto blocksize = sample_count > options.blocksize ? options.blocksize : sample_count;
    vm.SetParameters(params);
    vm.Execute(in.data(), out.data(), blocksize);
    sample_writer.Write(out.data(), blocksize);
    sample_count -= blocksize;
  }

  close(ofile);
  VERBOSE("** %zu samples written at %hu bits (blocksize %zu)", options.sample_count,
          options.bits_per_sample, options.blocksize);

  return EXIT_SUCCESS;
}
