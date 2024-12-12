/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 dlinear
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/parser/mps/SenseType.h"

using delpi::mps::ParseSense;
using delpi::mps::SenseType;

TEST(TestSense, ParseSense) {
  EXPECT_EQ(ParseSense("L"), SenseType::L);
  EXPECT_EQ(ParseSense("E"), SenseType::E);
  EXPECT_EQ(ParseSense("G"), SenseType::G);
  EXPECT_EQ(ParseSense("N"), SenseType::N);
}

TEST(TestSense, ParseSenseCaseInsensitive) {
  EXPECT_EQ(ParseSense("l"), SenseType::L);
  EXPECT_EQ(ParseSense("e"), SenseType::E);
  EXPECT_EQ(ParseSense("g"), SenseType::G);
  EXPECT_EQ(ParseSense("n"), SenseType::N);
}

TEST(TestSense, ParseSenseChar) {
  EXPECT_EQ(ParseSense('L'), SenseType::L);
  EXPECT_EQ(ParseSense('E'), SenseType::E);
  EXPECT_EQ(ParseSense('G'), SenseType::G);
  EXPECT_EQ(ParseSense('N'), SenseType::N);
}

TEST(TestSense, ParseSenseCharCaseInsensitive) {
  EXPECT_EQ(ParseSense('l'), SenseType::L);
  EXPECT_EQ(ParseSense('e'), SenseType::E);
  EXPECT_EQ(ParseSense('g'), SenseType::G);
  EXPECT_EQ(ParseSense('n'), SenseType::N);
}
