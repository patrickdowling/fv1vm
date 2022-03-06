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

#ifndef TEST_VM_H_
#define TEST_VM_H_

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "fv1/debug/fv1_debug.h"
#include "misc/program_stream.h"
#include "vm/vm.h"

namespace fv1 {

inline std::ostream &operator<<(std::ostream &os, OPCODE opcode)
{
  os << fv1::debug::to_string(opcode);
  return os;
}

template <typename T>
inline bool operator==(const AudioFrameT<T> &lhs, const AudioFrameT<T> &rhs)
{
  return lhs.l == rhs.l && lhs.r == rhs.r;
}
}  // namespace fv1

namespace fv1tests {

template <typename Engine, typename DelayStorage, size_t num_frames>
class TestVMImpl : public ::testing::Test {
public:
  using VM = typename fv1::VM<Engine, DelayStorage>;

  virtual void SetUp() final {}
  virtual void TearDown() final {}

protected:
  static constexpr size_t kNumFrames = num_frames;

  typename VM::DelayMemoryBuffer delay_memory_;
  fv1::BinaryProgramBuffer buffer_;
  VM vm_{delay_memory_};

  typename VM::AudioFrame in[kNumFrames];
  typename VM::AudioFrame out[kNumFrames];
  typename VM::Parameters params;

  void Compile(const char *filename)
  {
    static const std::string ROOT_PATH{"./build/tests/"};
    std::string path = ROOT_PATH + filename;

    using namespace fv1;
    int fd = open(path.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << path;
    auto bytes_read = read(fd, buffer_.data(), buffer_.size());
    close(fd);
    ASSERT_EQ(bytes_read, (ssize_t)buffer_.size());

    BufferStream<BSWAP_ENABLE> stream{buffer_.data()};
    vm_.Compile(stream);
  }
};

}  // namespace fv1tests

#endif  // TEST_VM_H_
