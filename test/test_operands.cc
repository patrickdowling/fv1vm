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

#include "fv1/fv1_instruction.h"

namespace testfv1 {

template <typename FP>
class TestFixedPointOperand : public ::testing::Test {
  static_assert(core::is_fixed_point<FP>());
};

using FPTypes =
    ::testing::Types<fv1::S4F6, fv1::SF10, fv1::S1F9, fv1::S1F14, fv1::I16, fv1::SF15, fv1::SF23>;

TYPED_TEST_SUITE(TestFixedPointOperand, FPTypes, );

TYPED_TEST(TestFixedPointOperand, is_fixed_point)
{
  auto operand = fv1::TypedOperand(TypeParam{1});
  EXPECT_TRUE(operand.is_fixed_point());

  EXPECT_EQ(operand.value, 1 << (fv1::SF23::FRAC - TypeParam::FRAC));
}

TEST(TestOperand, NotFixedPoint)
{
  {
    auto operand = fv1::TypedOperand{};
    EXPECT_FALSE(operand.is_fixed_point());
    EXPECT_TRUE(operand.is_none());
  }
  {
    auto operand = fv1::TypedOperand{fv1::kValueTag, 0};
    EXPECT_FALSE(operand.is_fixed_point());
    EXPECT_FALSE(operand.is_none());
    EXPECT_TRUE(operand.is_value());
  }
  {
    auto operand = fv1::TypedOperand{fv1::kMaskTag, 0};
    EXPECT_FALSE(operand.is_fixed_point());
    EXPECT_FALSE(operand.is_none());
    EXPECT_TRUE(operand.is_mask());
  }
  {
    auto operand = fv1::TypedOperand{fv1::kRegisterTag, 0};
    EXPECT_FALSE(operand.is_fixed_point());
    EXPECT_FALSE(operand.is_none());
    EXPECT_TRUE(operand.is_register());
  }
  {
    auto operand = fv1::TypedOperand{fv1::kAddrTag, 0};
    EXPECT_FALSE(operand.is_fixed_point());
    EXPECT_FALSE(operand.is_none());
    EXPECT_TRUE(operand.is_addr());
  }
}

}  // namespace testfv1

