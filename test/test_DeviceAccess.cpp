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
    EXPECT_THROW(DeviceAccess(""), libsarus::Error);

    //string longer than 3 characters
    EXPECT_THROW(DeviceAccess("rwma"), libsarus::Error);

    // characters outside 'rwm'
    EXPECT_THROW(DeviceAccess("rwa"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("zw"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("rpm"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("r&m"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("2w"), libsarus::Error);

    // repeated characters
    EXPECT_THROW(DeviceAccess("rr"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("rrr"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("rww"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("rwr"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("wmm"), libsarus::Error);

    // capitals of valid characters
    EXPECT_THROW(DeviceAccess("R"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("W"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("M"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("RW"), libsarus::Error);
    EXPECT_THROW(DeviceAccess("RWM"), libsarus::Error);
}

}}
