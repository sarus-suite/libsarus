/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <type_traits>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

#include "Error.hpp"
#include "Lockfile.hpp"
#include "Utility.hpp"


namespace libsarus {
namespace test {

class LockfileTest : public testing::Test {
protected:
    boost::filesystem::path fileToLock = libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/file-to-lock");
    boost::filesystem::path lockfile = fileToLock.string() + ".lock";
};

TEST_F(LockfileTest, creation_of_physical_lockfile) {
    EXPECT_FALSE(boost::filesystem::exists(lockfile));
    libsarus::Lockfile lock{fileToLock};
    EXPECT_TRUE(boost::filesystem::exists(lockfile));
}

TEST_F(LockfileTest, lock_acquisition) {
    {
        libsarus::Lockfile lock{fileToLock};
    }
    {
        // check that we can reacquire the lock
        // (previous lock was released when went out of scope)
        libsarus::Lockfile lock{fileToLock};
    }
    {
        libsarus::Lockfile lock{fileToLock};
        // check that lock cannot be acquired more than once
        EXPECT_THROW(libsarus::Lockfile(fileToLock, 0), libsarus::Error);
        // even if we try again...
        EXPECT_THROW(libsarus::Lockfile(fileToLock, 0), libsarus::Error);
    }
}

TEST_F(LockfileTest, move_constructor) {
    libsarus::Lockfile original{fileToLock};
    {
        libsarus::Lockfile moveConstructed{std::move(original)};
        // check that lock cannot be acquired more than once (move constructed lock is still active)
        EXPECT_THROW(libsarus::Lockfile(fileToLock, 0), libsarus::Error);
    }
    // check that lock can be acquired (move-constructed lock went out of scope)
    libsarus::Lockfile newlock{fileToLock};
}

TEST_F(LockfileTest, move_assignment) {
    libsarus::Lockfile original{fileToLock};
    {
        libsarus::Lockfile moveAssigned;
        moveAssigned = std::move(original);
        // check that lock cannot be acquired more than once (move assigned lock is still active)
        EXPECT_THROW(libsarus::Lockfile(fileToLock, 0), libsarus::Error);
    }
    // check that lock can be acquired (move-assigned lock went out of scope)
    libsarus::Lockfile newlock{fileToLock};
}

static_assert(!std::is_copy_constructible<libsarus::Lockfile>::value, "");
static_assert(!std::is_copy_assignable<libsarus::Lockfile>::value, "");
static_assert(std::is_move_constructible<libsarus::Lockfile>::value, "");
static_assert(std::is_move_assignable<libsarus::Lockfile>::value, "");

}}
