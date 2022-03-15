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

#include "fv1/fv1_asm_decode.h"

namespace testfv1 {

using fv1::detail::BitField;

struct BBBB {
  static constexpr std::string_view STRING = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";
};
struct A {
  static constexpr std::string_view STRING = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
};
struct ABBB {
  static constexpr std::string_view STRING = "ABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";
};
struct AAAA {
  static constexpr std::string_view STRING = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
};

TEST(TestBitField, Invalid)
{
  auto no_match = BitField::FromString<'A', BBBB>();
  EXPECT_EQ(0U, no_match.width);
  EXPECT_EQ(0U, no_match.shift);
}

TEST(TestBitField, Head)
{
  {
    auto bitfield = BitField::FromString<'A', A>();
    EXPECT_EQ(32U, bitfield.width);
    EXPECT_EQ(0U, bitfield.shift);
  }

  {
    auto bitfield = BitField::FromString<'A', ABBB>();
    EXPECT_EQ(1U, bitfield.width);
    EXPECT_EQ(31U, bitfield.shift);
  }

  {
    auto bitfield = BitField::FromString<'A', AAAA>();
    EXPECT_EQ(32U, bitfield.width);
    EXPECT_EQ(0U, bitfield.shift);
  }
}

}  // namespace testfv1
