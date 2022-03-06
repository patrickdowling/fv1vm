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

#include <gtest/gtest.h>

#include "fv1/fv1_delay_memory.h"
#include "vm/engines/delay_i32.h"

namespace fv1tests {

using fv1::engine::DelayStorageI32;

class TestDelayMemory : public ::testing::Test {
public:
  using DelayMemory = fv1::DelayMemory<DelayStorageI32>;

  virtual void SetUp() final { delay_memory_.Reset(); }
  virtual void TearDown() final {}

protected:
  DelayMemory::buffer_type buffer_;
  DelayMemory delay_memory_{buffer_};
};

TEST_F(TestDelayMemory, Reset)
{
  for (int32_t i = 0; i < fv1::kDelayMemorySize; ++i) { EXPECT_EQ(delay_memory_.Load(i), 0); }
}

TEST_F(TestDelayMemory, LoadStore)
{
  for (int32_t i = 0; i < fv1::kDelayMemorySize; ++i) delay_memory_.Store(i, fv1::SF23{i});

  auto last = delay_memory_.Load(0);
  for (int32_t i = 1; i < fv1::kDelayMemorySize; ++i) {
    auto value = delay_memory_.Load(i);
    EXPECT_EQ(value, i);
    EXPECT_NE(value, last);
  }
}
// if a 20 sample delay is desired it can be implemented by writing to address 0 and reading from
// address 20.
TEST_F(TestDelayMemory, Tick)
{
  static constexpr int32_t kDelayLength = 20;

  delay_memory_.Store(0, fv1::SF23{1234});
  EXPECT_EQ(delay_memory_.Load(0), 1234);
  EXPECT_EQ(delay_memory_.Load(kDelayLength), 0);

  int32_t ticks = kDelayLength - 1;
  while (ticks--) {
    delay_memory_.Tick();
    EXPECT_EQ(delay_memory_.Load(kDelayLength), 0);
  }
  delay_memory_.Tick();
  EXPECT_EQ(delay_memory_.Load(kDelayLength), 1234);
}

}  // namespace fv1tests
