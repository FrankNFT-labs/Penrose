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

#include <gtest/gtest.h>
#include <cstdint>
#include "drivers/debounce.h"

namespace penrose::test
{

// 12 buttons, a state change needs 4 consecutive identical scans.
using Debouncer = button::Debouncer<12, 4>;

class DebounceTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        debouncer.Init();
    }

    // Feed `count` identical raw readings to button `n` and return how many
    // press edges were reported.
    int Feed(uint8_t n, bool raw, int count)
    {
        int presses = 0;
        for (int i = 0; i < count; i++)
        {
            presses += debouncer.Update(n, raw) ? 1 : 0;
        }
        return presses;
    }

    Debouncer debouncer;
};

TEST_F(DebounceTest, InitiallyReleased)
{
    EXPECT_EQ(0u, debouncer.GetPressedStates());
    EXPECT_EQ(0, Feed(0, false, 10));
    EXPECT_FALSE(debouncer.IsPressed(0));
}

TEST_F(DebounceTest, PressRegistersOnFourthConsecutiveScan)
{
    EXPECT_EQ(0, Feed(5, true, 3));
    EXPECT_FALSE(debouncer.IsPressed(5));
    EXPECT_TRUE(debouncer.Update(5, true));
    EXPECT_TRUE(debouncer.IsPressed(5));
}

TEST_F(DebounceTest, HeldButtonReportsOneEdge)
{
    EXPECT_EQ(1, Feed(5, true, 100));
    EXPECT_TRUE(debouncer.IsPressed(5));
}

TEST_F(DebounceTest, BounceOnPressIsIgnored)
{
    // Contact chatter, then a stable press. Exactly one edge, on the fourth
    // consecutive closed reading.
    const bool readings[] = {true, false, true, false, true, true, true, true};
    int presses = 0;
    int edge_index = -1;
    int index = 0;
    for (bool raw : readings)
    {
        if (debouncer.Update(5, raw))
        {
            presses++;
            edge_index = index;
        }
        index++;
    }
    EXPECT_EQ(1, presses);
    EXPECT_EQ(7, edge_index);
    EXPECT_TRUE(debouncer.IsPressed(5));
}

TEST_F(DebounceTest, ReleaseRequiresFourConsecutiveScans)
{
    Feed(5, true, 4);
    EXPECT_EQ(0, Feed(5, false, 3));
    EXPECT_TRUE(debouncer.IsPressed(5));
    EXPECT_FALSE(debouncer.Update(5, false));
    EXPECT_FALSE(debouncer.IsPressed(5));
}

TEST_F(DebounceTest, BounceOnReleaseIsIgnored)
{
    Feed(5, true, 4);
    const bool readings[] = {false, true, false, false, false, false};
    int presses = 0;
    for (bool raw : readings)
    {
        presses += debouncer.Update(5, raw) ? 1 : 0;
    }
    EXPECT_EQ(0, presses);
    EXPECT_FALSE(debouncer.IsPressed(5));
}

TEST_F(DebounceTest, PressAfterReleaseReportsNewEdge)
{
    Feed(5, true, 4);
    Feed(5, false, 4);
    EXPECT_EQ(1, Feed(5, true, 4));
}

TEST_F(DebounceTest, ButtonsAreIndependent)
{
    Feed(3, true, 4);
    EXPECT_TRUE(debouncer.IsPressed(3));
    EXPECT_FALSE(debouncer.IsPressed(7));
    EXPECT_EQ(1u << 3, debouncer.GetPressedStates());

    Feed(11, true, 4);
    EXPECT_EQ((1u << 3) | (1u << 11), debouncer.GetPressedStates());

    Feed(3, false, 4);
    EXPECT_EQ(1u << 11, debouncer.GetPressedStates());
}

TEST_F(DebounceTest, InitClearsHistoryAndState)
{
    Feed(2, true, 4);
    debouncer.Init();
    EXPECT_EQ(0u, debouncer.GetPressedStates());
    // The history restarted too: three readings are not enough.
    EXPECT_EQ(0, Feed(2, true, 3));
}

}
