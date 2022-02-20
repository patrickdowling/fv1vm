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

#ifndef CORE_TYPE_TAG_H_
#define CORE_TYPE_TAG_H_

#include <cstdint>

namespace core {

// Type Tags are an overly complex FOURCC :)

struct TypeTagString {
  const char value[5] = {0};
};

using Tag = uint32_t;

struct TypeTag {
  const Tag tag = 0;

  constexpr explicit TypeTag(Tag uint_tag) : tag{uint_tag} {}

  static constexpr Tag FromString(const char *s, size_t len)
  {
    uint32_t uint_tag = len ? static_cast<uint32_t>(s[0]) : 0;
    if (len > 1) uint_tag |= static_cast<uint32_t>(s[1]) << 8;
    if (len > 2) uint_tag |= static_cast<uint32_t>(s[2]) << 16;
    if (len > 3) uint_tag |= static_cast<uint32_t>(s[3]) << 24;
    return uint_tag;
  }

  constexpr TypeTagString str() const
  {
    return TypeTagString{{static_cast<char>(tag), static_cast<char>(tag >> 8),
                          static_cast<char>(tag >> 16), static_cast<char>(tag >> 24), 0}};
  }
};

static constexpr bool operator==(const TypeTag &lhs, const TypeTag &rhs)
{
  return lhs.tag == rhs.tag;
}

}  // namespace core

// Pollute the global namespace?

constexpr core::Tag operator"" _TTU(const char *s, size_t len)
{
  return core::TypeTag::FromString(s, len);
}

constexpr core::TypeTag operator"" _TT(const char *s, size_t len)
{
  return core::TypeTag{core::TypeTag::FromString(s, len)};
}

#endif  // CORE_TYPE_TAG_H_
