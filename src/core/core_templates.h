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

#ifndef CORE_TEMPLATES_H_
#define CORE_TEMPLATES_H_

#include <type_traits>

namespace core {

template <typename T, typename... Ts>
struct are_distinct_types
    : std::conjunction<std::negation<std::is_same<T, Ts>>..., are_distinct_types<Ts...>> {};

template <typename T>
struct are_distinct_types<T> : std::true_type {};

}  // namespace core

#endif  // CORE_TEMPLATES_H_
