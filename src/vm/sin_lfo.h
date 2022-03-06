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

#ifndef FV1_SINLFO_H_
#define FV1_SINLFO_H_

#include "fv1/fv1_opcodes.h"
#include "lfo.h"

namespace fv1 {

// Lots of tricks to play here
// https://www.musicdsp.org/en/latest/Synthesis/10-fast-sine-and-cosine-calculation.html
//
// "Valid values are in the range 0 to 511 for Kf which results in a frequency range of about 0 to
// 20Hz at a sample rate of 32,768Hz"
//
// Kf = (0, 512]
// F = (Kf x 32000) / (2**17 x 2pi) = 19Hz
//
// But we do have to keep track of the decimal point...
// rate: 9 bits << 14 = 23
// amp : 15 << 8 = 23
//
template <typename Engine>
class SinLfoImpl : public LfoBase<Engine> {
public:
  static constexpr int32_t kRateShift = 23 - 9;
  static constexpr int32_t kRangeShift = 23 - 15;
  using value_type = typename LfoBase<Engine>::LfoValue;

  SinLfoImpl(const typename Engine::Register *rate, const typename Engine::Register *range)
      : LfoBase<Engine>(rate, range)
  {}

  void Tick()
  {
    auto coeff = this->rate_->loadi() >> 8;
    cos_ = cos_ + (sin_ * coeff);  // core::SSAT<SF23>(cos_ + (sin_ * coeff));
    sin_ = sin_ - (cos_ * coeff);  // core::SSAT<SF23>(sin_ - (cos_ * coeff));
  }

  void Jam()
  {
    sin_.value = 0;
    cos_.value = SF23::MIN;
  }

  inline SF23 sin() const { return sin_ * this->range_->load(); }
  inline SF23 cos() const { return cos_ * this->range_->load(); }

  // VALID: (SIN) COS REG COMPC COMPA
  value_type Read(const CHO_FLAGS flags) const
  {
    int32_t v = ((CHO_FLAGS::COS & flags) ? cos() : sin()).value;
    auto coefficient = Engine::template LfoCoeffToFloat<8>(v & 0xff);
    if (CHO_FLAGS::COMPA & flags) v = -v;
    if (CHO_FLAGS::COMPC & flags) coefficient = Engine::ONE - coefficient;
    return value_type{v >> 8, coefficient};
  }

private:
  SF23 sin_{0};
  SF23 cos_{SF23::MIN};
};

}  // namespace fv1

#endif  // FV1_SINLFO_H_
