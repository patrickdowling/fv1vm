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

#ifndef FV1_TOOLS_H_
#define FV1_TOOLS_H_

#include <cctype>
#include <cstdio>
#include <string>

#include "fv1/fv1_bank.h"

#define INFO(...)                         \
  do {                                    \
    fv1tools::logga(stdout, __VA_ARGS__); \
  } while (0)
#define ERR(...)                          \
  do {                                    \
    fv1tools::logga(stderr, __VA_ARGS__); \
  } while (0)

namespace fv1tools {

void logga(FILE *f, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
char *trim_str(char *str);
void print_program_info(const fv1::BankInfo *bank_info, int program_index);
void print_bank_type(size_t s);

class BinaryFile {
public:
  bool Read(const std::string &filename);

  char *buffer() { return buffer_; }

  size_t length() const { return length_; }

  bool valid_length() const
  {
    return length_ == 512 || length_ == 4096 || length_ >= fv1::kDervishBankSize;
  }

  const fv1::BankInfo *get_bank_info() const
  {
    return length_ < fv1::kDervishBankSize
               ? 0
               : reinterpret_cast<const fv1::BankInfo *>(buffer_ + 4096);
  }

  const char *program(int index) const
  {
    auto offset = (size_t)index * 512;
    return offset < length_ ? buffer_ + offset : nullptr;
  }

private:
  char buffer_[fv1::kDervishBankSize];
  size_t length_ = 0;
};

}  // namespace fv1tools

#endif // FV1_TOOLS_H_
