/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/util/math.h"

using delpi::Max;
using delpi::Min;

TEST(TestMath, MaxSingle) { EXPECT_EQ(Max(1), 1); }

TEST(TestMath, MaxSequence) { EXPECT_EQ(Max(1, 2, 3, 2, 1, -1), 3); }

TEST(TestMath, MinSingle) { EXPECT_EQ(Min(1), 1); }

TEST(TestMath, MinSequence) { EXPECT_EQ(Min(1, 2, 3, 2, 1, -1), -1); }
