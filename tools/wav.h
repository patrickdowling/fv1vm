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

#ifndef TOOLS_WAV_H_
#define TOOLS_WAV_H_

#include <cstdint>

namespace wav {

struct ChunkHeader {
  const uint8_t ID[4];
  uint32_t size;
};
struct Fmt {
  ChunkHeader header = {{'f', 'm', 't', ' '}, 16};
  uint16_t format_tag;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
} fmt;

struct WAVHeader {
  ChunkHeader riff_header = {{'R', 'I', 'F', 'F'}, 0};
  const uint8_t riff_type_id[4] = {'W', 'A', 'V', 'E'};

  Fmt fmt;
  ChunkHeader data = {{'d', 'a', 't', 'a'}, 0};
};
static_assert(sizeof(WAVHeader) == 44);
static_assert(sizeof(Fmt) == 16 + sizeof(ChunkHeader));

class SampleWriter {
public:
  SampleWriter(int fd, size_t bits_per_sample) : fd_{fd}, bytes_per_sample_{bits_per_sample / 8} {}

  template <typename Frame>
  ssize_t Write(const Frame *samples, size_t count)
  {
    ssize_t s = 0;
    while (count--) {
      int32_t l = samples->l;
      int32_t r = samples->r;
      switch (bytes_per_sample_) {
        case 2:
          l >>= 8;
          r >>= 8;
          break;
        case 3: break;
      }
      s += write(fd_, &l, bytes_per_sample_);
      s += write(fd_, &r, bytes_per_sample_);
      ++samples;
    }
    return s;
  }

private:
  int fd_ = -1;
  size_t bytes_per_sample_ = 16;
};

}  // namespace wav

#endif // TOOLS_WAV_H_
