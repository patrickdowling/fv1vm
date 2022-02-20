// fv1vm: experimental fv-1 virtual machine
// copyright (c) 2022 patrick dowling <pld@gurkenkiste.com>
//
// this program is free software: you can redistribute it and/or modify
// it under the terms of the gnu general public license as published by
// the free software foundation, either version 3 of the license, or
// (at your option) any later version.
//
// this program is distributed in the hope that it will be useful,
// but without any warranty; without even the implied warranty of
// merchantability or fitness for a particular purpose.  see the
// gnu general public license for more details.
//
// you should have received a copy of the gnu general public license
// along with this program.  if not, see <https://www.gnu.org/licenses/>.

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
