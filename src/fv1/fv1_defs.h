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

#ifndef FV1_DEFS_H_
#define FV1_DEFS_H_

#include <array>
#include <cstdint>
#include <cstring>

#include "core/core_fixed_point.h"
#include "core/core_type_tag.h"

namespace fv1 {
static constexpr int32_t kMaxInstructionCount = 128;
static constexpr int32_t kDelayMemorySize = 32768;
static constexpr int32_t kNumPots = 3;
static constexpr int32_t kMaxOperands = 3;

static constexpr int32_t kDelayAddrMask = 0x7fff;
static_assert(kDelayAddrMask == kDelayMemorySize - 1);

using BinaryProgramBuffer = std::array<char, sizeof(uint32_t) * kMaxInstructionCount>;
static_assert(512 == sizeof(BinaryProgramBuffer));

FIXED_POINT_TYPE(S4F6, 11, 6);
FIXED_POINT_TYPE(SF10, 11, 10);
FIXED_POINT_TYPE(S1F9, 11, 9);
FIXED_POINT_TYPE_S(S1F14, "S114", 16, 14);
FIXED_POINT_TYPE(I16,  16, 15);
FIXED_POINT_TYPE(SF15, 16, 15);
FIXED_POINT_TYPE(SF23, 24, 23);

}  // namespace fv1

#endif  // FV1_DEFS_H_
