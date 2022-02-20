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

#ifndef FV1_BANK_H_
#define FV1_BANK_H_

#include <cstdint>
#include <cstring>

namespace fv1 {

static constexpr size_t kStrLen = 21;

// Bank structure from Dervish EEPROM banks
// They are stored following the 4K of a bank and so aren't headers.

struct ProgramInfo {
  char name[kStrLen] = {0};
  char pot0[kStrLen] = {0};
  char pot1[kStrLen] = {0};
  char pot2[kStrLen] = {0};
};
static_assert(sizeof(ProgramInfo) == 84);

struct BankInfo {
  char name[kStrLen] = {0};

  ProgramInfo programs[8];
};
static_assert(sizeof(BankInfo) == 21 + 8 * sizeof(ProgramInfo));

static constexpr size_t kDervishBankSize = 8 * 512 + sizeof(BankInfo);

}  // namespace fv1

#endif // FV1_BANK_H_
