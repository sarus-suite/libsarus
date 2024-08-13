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

#include "Error.hpp"

#define _DECLARE(tag) int __lineno_##tag = -1;
#define _TAG(tag, expr)            \
    do {                           \
        __lineno_##tag = __LINE__; \
        expr;                      \
    } while (0)
#define _GET(tag) (__lineno_##tag)

namespace libsarus {
namespace test {

class ErrorTestGroup : public testing::Test {
  protected:
    _DECLARE(FTT)
    _DECLARE(FTR)
    _DECLARE(FTTFSE)
    _DECLARE(FTTWLLD)
    _DECLARE(FTRWLLD)

    void functionThatThrows() {
        _TAG(FTT, SARUS_THROW_ERROR("first error message"));
    }

    void functionThatRethrows() {
        try {
            functionThatThrows();
        } catch (libsarus::Error& error) {
            _TAG(FTR, SARUS_RETHROW_ERROR(error, "second error message"));
        }
    }

    void functionThatThrowsFromStdException() {
        auto stdException = std::runtime_error("first error message");
        const auto& ref = stdException;
        _TAG(FTTFSE, SARUS_RETHROW_ERROR(ref, "second error message"));
    }

    void functionThatThrowsWithLogLevelDebug() {
        _TAG(FTTWLLD, SARUS_THROW_ERROR("first error message",
                                        libsarus::LogLevel::DEBUG));
    }

    void functionThatRethrowsWithLogLevelDebug() {
        try {
            functionThatThrows();
        } catch (libsarus::Error& error) {
            _TAG(FTRWLLD, SARUS_RETHROW_ERROR(error, "second error message",
                                              libsarus::LogLevel::DEBUG));
        }
    }
};

TEST_F(ErrorTestGroup, oneStackTraceEntry) {
    try {
        functionThatThrows();
    } catch (const libsarus::Error& error) {
        auto expectedFirstEntry = libsarus::Error::ErrorTraceEntry{
            "first error message", "test_Error.cpp", _GET(FTT),
            "functionThatThrows"};

        EXPECT_EQ(error.getErrorTrace().size(), 1);
        EXPECT_EQ(error.getErrorTrace()[0], expectedFirstEntry);
        EXPECT_EQ(error.getLogLevel(), libsarus::LogLevel::ERROR);
    }
}

TEST_F(ErrorTestGroup, twoStackTraceEntries) {
    try {
        functionThatRethrows();
    } catch (const libsarus::Error& error) {
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

TEST_F(ErrorTestGroup, fromStdException) {
    try {
        functionThatThrowsFromStdException();
    } catch (const libsarus::Error& error) {
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

TEST_F(ErrorTestGroup, oneStackTraceEntry_throwWithLogLevelDebug) {
    try {
        functionThatThrowsWithLogLevelDebug();
    } catch (const libsarus::Error& error) {
        auto expectedFirstEntry = libsarus::Error::ErrorTraceEntry{
            "first error message", "test_Error.cpp", _GET(FTTWLLD),
            "functionThatThrowsWithLogLevelDebug"};

        EXPECT_EQ(error.getErrorTrace().size(), 1);
        EXPECT_EQ(error.getErrorTrace()[0], expectedFirstEntry);
        EXPECT_EQ(error.getLogLevel(), libsarus::LogLevel::DEBUG);
    }
}

TEST_F(ErrorTestGroup, twoStackTraceEntries_rethrowWithLogLevelDebug) {
    try {
        functionThatRethrowsWithLogLevelDebug();
    } catch (const libsarus::Error& error) {
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
