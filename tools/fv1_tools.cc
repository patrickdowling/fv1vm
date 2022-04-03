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

#include "fv1_tools.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstdarg>
#include <cstdio>

namespace fv1tools {

void logga(FILE *f, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  char buffer[1024] = {0};
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  fprintf(f, "%s\n", buffer);
}

void print_program_info(const fv1::BankInfo *bank_info, int program_index)
{
  if (bank_info) {
    INFO("BANK   : '%.20s'", bank_info->name);
    INFO(" PROG %d: '%.20s'", program_index, bank_info->programs[program_index].name);
    INFO("   POT0: '%.20s'", bank_info->programs[program_index].pot0);
    INFO("   POT1: '%.20s'", bank_info->programs[program_index].pot1);
    INFO("   POT2: '%.20s'", bank_info->programs[program_index].pot2);
  }
}

void print_bank_type(size_t size)
{
  if (512 == size)
    INFO("** %zu bytes, single program", size);
  else if (4096 == size)
    INFO("** %zu bytes, program bank", size);
  else if (size >= fv1::kDervishBankSize)
    INFO("** %zu bytes, Dervish bank", size);
}

bool BinaryFile::Read(const std::string &filename)
{
  int fd = STDIN_FILENO;
  if (!filename.empty()) {
    fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) { return false; }
  }
  auto bytes_read = read(fd, buffer_, sizeof(buffer_));
  length_ = bytes_read >= 0 ? (size_t)bytes_read : 0;

  if (STDIN_FILENO != fd) close(fd);
  return length_ > 0;
}

}  // namespace fv1tools
