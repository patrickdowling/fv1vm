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

#ifndef FV1_PROGRAM_STREAM_H_
#define FV1_PROGRAM_STREAM_H_

#include <cstdint>
#include <cstring>

namespace fv1 {

enum BSWAP { BSWAP_DISABLE, BSWAP_ENABLE };

class ProgramStream {
public:
  virtual ~ProgramStream();

  virtual bool available() const = 0;
  virtual void Reset() = 0;

  uint32_t Next()
  {
    uint32_t i = ReadNext();
    return bswap_ ? __builtin_bswap32(i) : i;
  }

protected:
  const BSWAP bswap_;

  ProgramStream(BSWAP bswap) : bswap_(bswap) {}

  virtual uint32_t ReadNext() = 0;
};

template <BSWAP bswap>
class BufferStream : public ProgramStream {
public:
  BufferStream(const char *buffer)
      : ProgramStream(bswap), buffer_{buffer}, head_{buffer}, tail_{buffer + sizeof(uint32_t) * 128}
  {}

  virtual bool available() const final { return head_ < tail_; }
  virtual void Reset() final { head_ = buffer_; }

protected:
  const char *buffer_;
  const char *head_;
  const char *tail_;

  virtual uint32_t ReadNext() final
  {
    uint32_t i;
    std::memcpy(&i, head_, sizeof(uint32_t));
    head_ += sizeof(uint32_t);
    return i;
  }
};

}  // namespace fv1

#endif  // FV1_PROGRAM_STREAM_H_
