/*
Mixer file header
Copyright (C) 2025  Vincent Lavoie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <common.hh>

#define MIXER_SAMPLE_LOW 32768
#define MIXER_SAMPLE_HI 65536

namespace mixer
{
typedef short sample;

constexpr inline mixer::sample MixSamples(const mixer::sample LeftSample,
                                          const mixer::sample RightSample)
{
  // Taken from Viktor T. Toth formula for mixing PCM audio
  // http://www.vttoth.com/CMS/index.php/technical-notes/68
  // store in temporary i32 to avoid overflow/underflow
  i32 Left = LeftSample;
  i32 Right = RightSample;
  i32 MixedSample = 0;

  Left += MIXER_SAMPLE_LOW;
  Right += MIXER_SAMPLE_LOW;

  // both sources are low frequency
  if ((Left < MIXER_SAMPLE_LOW) || (Right < MIXER_SAMPLE_LOW))
  {
    MixedSample = Left * Right / MIXER_SAMPLE_LOW;
  }
  else
  {
    // else one of the sources is high frequency
    MixedSample = 2 * (Left + Right) - (Left * Right) / MIXER_SAMPLE_LOW - MIXER_SAMPLE_HI;
  }

  if (MixedSample == MIXER_SAMPLE_HI)
  {
    MixedSample = MIXER_SAMPLE_HI - 1;
  }

  MixedSample -= MIXER_SAMPLE_LOW;

  return (mixer::sample)MixedSample;
}
} // namespace mixer
