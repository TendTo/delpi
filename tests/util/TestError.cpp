/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include <stdexcept>

#include "delpi/util/error.h"

using delpi::DelpiAssertionException;
using delpi::DelpiException;
using delpi::DelpiInvalidArgumentException;
using delpi::DelpiUnreachableException;
using std::invalid_argument;
using std::runtime_error;

#ifndef NDEBUG

TEST(TestException, AssertFail) { EXPECT_THROW(DELPI_ASSERT(false, "Message"), DelpiAssertionException); }

TEST(TestException, AssertFailReport) { EXPECT_THROW(DELPI_ASSERT(1 + 1 == 3, "Message"), DelpiAssertionException); }

TEST(TestException, AssertSuccess) { EXPECT_NO_THROW(DELPI_ASSERT(true, "Message")); }

TEST(TestException, Unreachable) { EXPECT_THROW(DELPI_UNREACHABLE(), DelpiUnreachableException); }

#else

TEST(TestException, AssertFail) { EXPECT_NO_THROW(DELPI_ASSERT(false, "Message")); }

TEST(TestException, AssertFailReport) { EXPECT_NO_THROW(DELPI_ASSERT(1 + 1 == 3, "Message")); }

TEST(TestException, AssertSuccess) { EXPECT_NO_THROW(DELPI_ASSERT(true, "Message")); }

TEST(TestException, Unreachable) { EXPECT_DEATH(DELPI_UNREACHABLE()); }

#endif

TEST(TestException, RuntimeError) { EXPECT_THROW(DELPI_RUNTIME_ERROR("Message"), DelpiException); }

TEST(TestException, RuntimeErrorFmt) { EXPECT_THROW(DELPI_RUNTIME_ERROR_FMT("Message: {}", "format"), DelpiException); }

TEST(TestException, InvalidArgument) {
  EXPECT_THROW(DELPI_INVALID_ARGUMENT("Message: {}", "actual"), DelpiInvalidArgumentException);
}

TEST(TestException, InvalidArgumentExpected) {
  EXPECT_THROW(DELPI_INVALID_ARGUMENT_EXPECTED("Message: {}", "actual", "expected"), DelpiInvalidArgumentException);
}
