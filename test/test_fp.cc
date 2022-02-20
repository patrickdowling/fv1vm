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

#include <limits>

#include "fv1/fv1_defs.h"

namespace testfv1 {

TEST(TestFixedPoint, I16)
{
  using fv1::I16;
  EXPECT_EQ(std::numeric_limits<int16_t>::max(), I16::MAX);
  EXPECT_EQ(std::numeric_limits<int16_t>::min(), I16::MIN);

  EXPECT_EQ(0, core::DecodeFixedPoint<I16>(0).value);
  EXPECT_EQ(-16384, core::DecodeFixedPoint<I16>(-0x4000U).value);
  EXPECT_EQ(-8192, core::DecodeFixedPoint<I16>(0xe000).value);
  EXPECT_EQ(I16::MAX, core::DecodeFixedPoint<I16>(0x7fff).value);
  EXPECT_EQ(I16::MIN, core::DecodeFixedPoint<I16>(0x8000).value);
}

TEST(TestFixedPoint, SF23)
{
  using fv1::SF23;

  EXPECT_STREQ("SF23", SF23::TAG.str().value);

  EXPECT_EQ(0x007fffffU, (uint32_t)SF23::MAX);
  EXPECT_EQ(0xff800000U, (uint32_t)SF23::MIN);
  EXPECT_EQ(0x00ffffffU, (uint32_t)SF23::MASK);
  EXPECT_EQ(8388608.f, SF23::kFloatScale);

  EXPECT_GE(SF23{SF23::MAX}.to_float(), .99f);
  EXPECT_LE(SF23{SF23::MIN}.to_float(), -1.f);

  EXPECT_EQ(SF23::MIN, core::DecodeFixedPoint<SF23>(0x00800000U).value);
}

TEST(TestFixedPoint, S1F14)
{
  using fv1::S1F14;

  EXPECT_STREQ("S114", S1F14::TAG.str().value);

  EXPECT_GE(S1F14{S1F14::MAX}.to_float(), 1.99f);
  EXPECT_LE(S1F14{S1F14::MIN}.to_float(), -2.f);
}

TEST(TestFixedPoint, math_ideas)
{
  using fv1::SF23;

  int32_t a = fv1::SF23::MAX;
  int32_t b = fv1::SF23::MIN;

  auto i = static_cast<int32_t>((static_cast<int64_t>(a) * b) >> 24);
  auto j = static_cast<int32_t>((static_cast<int64_t>(a) * (b << 8)) >> 32);

  EXPECT_EQ(i, j);
}

}  // namespace testfv1
