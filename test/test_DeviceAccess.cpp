/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "DeviceAccess.hpp"

#include <gtest/gtest.h>


namespace libsarus {
namespace test {

class DeviceAccessTestGroup : public testing::Test {
protected:
};

TEST_F(DeviceAccessTestGroup, valid_inputs) {
    auto access = DeviceAccess("rwm");
    EXPECT_EQ(access.string(), "rwm");

    access = DeviceAccess("wmr");
    EXPECT_EQ(access.string(), "rwm");

    access = DeviceAccess("r");
    EXPECT_EQ(access.string(), "r");

    access = DeviceAccess("w");
    EXPECT_EQ(access.string(), "w");

    access = DeviceAccess("m");
    EXPECT_EQ(access.string(), "m");

    access = DeviceAccess("rw");
    EXPECT_EQ(access.string(), "rw");

    access = DeviceAccess("wr");
    EXPECT_EQ(access.string(), "rw");

    access = DeviceAccess("mr");
    EXPECT_EQ(access.string(), "rw");

    access = DeviceAccess("wm");
    EXPECT_EQ(access.string(), "wm");

    access = DeviceAccess("mw");
    EXPECT_EQ(access.string(), "wm");
}

TEST_F(DeviceAccessTestGroup, invalid_inputs) {
    // empty string
    CHECK_THROWS(libsarus::Error, DeviceAccess(""));

    //string longer than 3 characters
    CHECK_THROWS(libsarus::Error, DeviceAccess("rwma"));

    // characters outside 'rwm'
    CHECK_THROWS(libsarus::Error, DeviceAccess("rwa"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("zw"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("rpm"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("r&m"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("2w"));

    // repeated characters
    CHECK_THROWS(libsarus::Error, DeviceAccess("rr"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("rrr"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("rww"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("rwr"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("wmm"));

    // capitals of valid characters
    CHECK_THROWS(libsarus::Error, DeviceAccess("R"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("W"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("M"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("RW"));
    CHECK_THROWS(libsarus::Error, DeviceAccess("RWM"));
}

}}

