/*
 *  Copyright 2026 Frank Poncelet
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PENROSE_DRIVERS_DEBOUNCE_H_
#define PENROSE_DRIVERS_DEBOUNCE_H_

#include <stdint.h>

namespace penrose {
namespace button {

// Per-button debouncer. A button's debounced state changes only after
// kSamples consecutive scans agree, which filters mechanical contact bounce.
// The matrix scanner visits each button once every kNumButtons samples, so
// with 12 buttons at 8 kHz and 4 samples a press registers after about 6 ms
// of stable contact.
template <uint8_t kNumButtons, uint8_t kSamples = 4>
class Debouncer
{
    static_assert(kNumButtons >= 1 && kNumButtons <= 16,
                  "pressed state is 16 bits wide");
    static_assert(kSamples >= 1 && kSamples <= 8, "history is 8 bits wide");

  public:
    void Init(void)
    {
        for (uint8_t i = 0; i < kNumButtons; i++)
        {
            history_[i] = 0;
        }
        pressed_ = 0;
    }

    // Feed one raw reading for button `n`, true when the contact is closed.
    // Returns true on the scan where the debounced state becomes pressed.
    bool Update(uint8_t n, bool raw)
    {
        const uint8_t history =
            static_cast<uint8_t>((history_[n] << 1) | (raw ? 1 : 0)) & kMask;
        history_[n] = history;
        const uint16_t bit = static_cast<uint16_t>(1u << n);

        if (history == kMask)
        {
            const bool was_pressed = (pressed_ & bit) != 0;
            pressed_ |= bit;
            return !was_pressed;
        }

        if (history == 0)
        {
            pressed_ &= static_cast<uint16_t>(~bit);
        }

        return false;
    }

    bool IsPressed(uint8_t n) const
    {
        return (pressed_ & static_cast<uint16_t>(1u << n)) != 0;
    }

    uint16_t GetPressedStates(void) const
    {
        return pressed_;
    }

  private:
    static constexpr uint8_t kMask = static_cast<uint8_t>((1u << kSamples) - 1);

    uint8_t history_[kNumButtons];
    uint16_t pressed_;
};

}
}

#endif
