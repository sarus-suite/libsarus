/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <sstream>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

#include "Error.hpp"
#include "Lockfile.hpp"
#include "Logger.hpp"
#include "Utility.hpp"


namespace libsarus {
namespace test {

class CLIArgumentsTestGroup : public testing::Test {
protected:
};

TEST_F(CLIArgumentsTestGroup, serialize) {
    auto args = libsarus::CLIArguments{"command", "arg0", "arg1"};

    std::stringstream os;
    os << args;

    EXPECT_EQ(os.str(), std::string{"[\"command\", \"arg0\", \"arg1\"]"});
};

TEST_F(CLIArgumentsTestGroup, deserialize) {
    std::stringstream is("[\"command\", \"arg0\", \"arg1\"]");

    libsarus::CLIArguments args;
    is >> args;

    auto expected = libsarus::CLIArguments{"command", "arg0", "arg1"};
    EXPECT_EQ(args, expected);
};

TEST_F(CLIArgumentsTestGroup, string) {
    std::stringstream is("[\"command\", \"arg0\", \"arg1\"]");

    libsarus::CLIArguments args;
    is >> args;

    auto expected = std::string{"command arg0 arg1"};
    EXPECT_EQ(args.string(), expected);
};

}}
