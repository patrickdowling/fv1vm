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

#ifndef FV1_DELAY_MEMORY_H_
#define FV1_DELAY_MEMORY_H_

#include <algorithm>
#include <array>
#include <cstdint>

#include "fv1_defs.h"

// TODO struct Address {} for additional type safety

namespace fv1 {

// Generally assue offsets are >= 0
template <typename Traits>
class DelayMemory {
public:
  using value_type = typename Traits::value_type;
  using storage_type = typename Traits::storage_type;
  using buffer_type = typename std::array<typename Traits::storage_type, kDelayMemorySize>;

  explicit DelayMemory(buffer_type &buffer) : buffer_{buffer} {}

  void Reset()
  {
    std::fill(buffer_.begin(), buffer_.end(), 0);
    cursor_ = 0;
  }

  constexpr int32_t size() { return kDelayMemorySize; }

  value_type Load(int32_t index)
  {
    last_read_ = Traits::Unpack(at(index));
    return last_read_;
  }

  void Store(int32_t index, value_type value) { at(index) = Traits::Pack(value); }

  template <typename T>
  void Store(int32_t index, const T &value)
  {
    at(index) = Traits::Pack(value.load());
  }

  value_type last_read() const { return last_read_; }

  void Tick()
  {
    auto c = cursor_;
    if (c)
      cursor_ = c - 1;
    else
      cursor_ = kDelayMemorySize - 1;
  }

  value_type load_immediate(int32_t index) const { return Traits::Unpack(buffer_[index]); }

private:
  buffer_type &buffer_;
  int32_t cursor_{0};
  value_type last_read_{0};

  inline storage_type at(int32_t i) const
  {
    return buffer_[(cursor_ + i) & (kDelayMemorySize - 1)];
  }

  inline storage_type &at(int32_t i) { return buffer_[(cursor_ + i) & (kDelayMemorySize - 1)]; }
};

}  // namespace fv1

#endif  // FV1_DELAY_MEMORY_H_
