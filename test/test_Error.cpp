/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <exception>

#include <gtest/gtest.h>

#include "libsarus/Error.hpp"

// clang-format off
#define _DECLARE(tag) int __lineno_##tag = -1;
#define _TAG(tag, expr) do { __lineno_##tag = __LINE__; expr; } while (0)
#define _GET(tag) (__lineno_##tag)
// clang-format on

namespace libsarus {
namespace test {

class ErrorTest : public testing::Test {
  protected:
    _DECLARE(FTT)
    _DECLARE(FTR)
    _DECLARE(FTTFSE)
    _DECLARE(FTTWLLD)
    _DECLARE(FTRWLLD)

    void functionThatThrows() {
        // clang-format off
        _TAG(FTT, SARUS_THROW_ERROR("first error message"));
        // clang-format on
    }

    void functionThatRethrows() {
        try {
            functionThatThrows();
        } catch (libsarus::Error &error) {
            // clang-format off
            _TAG(FTR, SARUS_RETHROW_ERROR(error, "second error message"));
            // clang-format on
        }
    }

    void functionThatThrowsFromStdException() {
        auto stdException = std::runtime_error("first error message");
        const auto &ref = stdException;
        // clang-format off
        _TAG(FTTFSE, SARUS_RETHROW_ERROR(ref, "second error message"));
        // clang-format on
    }

    void functionThatThrowsWithLogLevelDebug() {
        // clang-format off
        _TAG(FTTWLLD, SARUS_THROW_ERROR("first error message", libsarus::LogLevel::DEBUG));
        // clang-format on
    }

    void functionThatRethrowsWithLogLevelDebug() {
        try {
            functionThatThrows();
        } catch (libsarus::Error &error) {
            // clang-format off
            _TAG(FTRWLLD, SARUS_RETHROW_ERROR(error, "second error message", libsarus::LogLevel::DEBUG));
            // clang-format on
        }
    }
};

TEST_F(ErrorTest, oneStackTraceEntry) {
    try {
        functionThatThrows();
    } catch (const libsarus::Error &error) {
        auto expectedFirstEntry = libsarus::Error::ErrorTraceEntry{
            "first error message", "test_Error.cpp", _GET(FTT),
            "functionThatThrows"};

        EXPECT_EQ(error.getErrorTrace().size(), 1);
        EXPECT_EQ(error.getErrorTrace()[0], expectedFirstEntry);
        EXPECT_EQ(error.getLogLevel(), libsarus::LogLevel::ERROR);
    }
}

TEST_F(ErrorTest, twoStackTraceEntries) {
    try {
        functionThatRethrows();
    } catch (const libsarus::Error &error) {
        auto expectedFirstEntry = libsarus::Error::ErrorTraceEntry{
            "first error message", "test_Error.cpp", _GET(FTT),
            "functionThatThrows"};
        auto expectedSecondEntry = libsarus::Error::ErrorTraceEntry{
            "second error message", "test_Error.cpp", _GET(FTR),
            "functionThatRethrows"};

        EXPECT_EQ(error.getErrorTrace().size(), 2);
        EXPECT_EQ(error.getErrorTrace()[0], expectedFirstEntry);
        EXPECT_EQ(error.getErrorTrace()[1], expectedSecondEntry);
        EXPECT_EQ(error.getLogLevel(), libsarus::LogLevel::ERROR);
    }
}

TEST_F(ErrorTest, fromStdException) {
    try {
        functionThatThrowsFromStdException();
    } catch (const libsarus::Error &error) {
        auto expectedFirstEntry = libsarus::Error::ErrorTraceEntry{
            "first error message", "unspecified location", -1, "runtime error"};
        auto expectedSecondEntry = libsarus::Error::ErrorTraceEntry{
            "second error message", "test_Error.cpp", _GET(FTTFSE),
            "functionThatThrowsFromStdException"};

        EXPECT_EQ(error.getErrorTrace().size(), 2);
        EXPECT_EQ(error.getErrorTrace()[0], expectedFirstEntry);
        EXPECT_EQ(error.getErrorTrace()[1], expectedSecondEntry);
        EXPECT_EQ(error.getLogLevel(), libsarus::LogLevel::ERROR);
    }
}

TEST_F(ErrorTest, oneStackTraceEntry_throwWithLogLevelDebug) {
    try {
        functionThatThrowsWithLogLevelDebug();
    } catch (const libsarus::Error &error) {
        auto expectedFirstEntry = libsarus::Error::ErrorTraceEntry{
            "first error message", "test_Error.cpp", _GET(FTTWLLD),
            "functionThatThrowsWithLogLevelDebug"};

        EXPECT_EQ(error.getErrorTrace().size(), 1);
        EXPECT_EQ(error.getErrorTrace()[0], expectedFirstEntry);
        EXPECT_EQ(error.getLogLevel(), libsarus::LogLevel::DEBUG);
    }
}

TEST_F(ErrorTest, twoStackTraceEntries_rethrowWithLogLevelDebug) {
    try {
        functionThatRethrowsWithLogLevelDebug();
    } catch (const libsarus::Error &error) {
        auto expectedFirstEntry = libsarus::Error::ErrorTraceEntry{
            "first error message", "test_Error.cpp", _GET(FTT),
            "functionThatThrows"};
        auto expectedSecondEntry = libsarus::Error::ErrorTraceEntry{
            "second error message", "test_Error.cpp", _GET(FTRWLLD),
            "functionThatRethrowsWithLogLevelDebug"};

        EXPECT_EQ(error.getErrorTrace().size(), 2);
        EXPECT_EQ(error.getErrorTrace()[0], expectedFirstEntry);
        EXPECT_EQ(error.getErrorTrace()[1], expectedSecondEntry);
        EXPECT_EQ(error.getLogLevel(), libsarus::LogLevel::DEBUG);
    }
}

}  // namespace test
}  // namespace libsarus
