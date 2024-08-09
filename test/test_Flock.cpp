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
#include "Flock.hpp"
#include "Utility.hpp"


namespace libsarus {
namespace test {

constexpr std::chrono::milliseconds operator""_ms(unsigned long long ms) {
    return std::chrono::milliseconds(ms);
}

static bool lockAcquisitionDoesntThrow(const boost::filesystem::path &fileToLock, const libsarus::Flock::Type &lockType) {
    try {
        libsarus::Flock{fileToLock, lockType, 10_ms};
    } catch (const libsarus::Error &) {
        return false;
    }
    return true;
}

class FlockTestGroup : public testing::Test {
protected:
    boost::filesystem::path fileToLock = libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/file-to-lock");
    boost::filesystem::path lockfile = fileToLock.string() + ".lock";

    FlockTestGroup() {
        libsarus::filesystem::createFileIfNecessary(fileToLock);
    }

    ~FlockTestGroup() override {
        boost::filesystem::remove(fileToLock);
    }
};

TEST_F(FlockTestGroup, lock_is_released_when_the_object_is_destroyed) {
    {
        libsarus::Logger::getInstance().setLevel(libsarus::LogLevel::DEBUG);
        libsarus::Flock lock{fileToLock, libsarus::Flock::Type::writeLock};
    }
    {
        CHECK(lockAcquisitionDoesntThrow(fileToLock, libsarus::Flock::Type::writeLock));
    }
    {
        CHECK(lockAcquisitionDoesntThrow(fileToLock, libsarus::Flock::Type::writeLock));
    }
}

TEST_F(FlockTestGroup, move_constructor_moves_resources) {
    libsarus::Flock original{fileToLock, libsarus::Flock::Type::writeLock};
    {
        libsarus::Flock moveConstructed{std::move(original)};
        CHECK_THROWS(libsarus::Error, libsarus::Flock(fileToLock, libsarus::Flock::Type::writeLock, 10_ms));
    }
    // check that lock can be acquired (move-constructed lock went out of scope)
    CHECK(lockAcquisitionDoesntThrow(fileToLock, libsarus::Flock::Type::writeLock));
}

TEST_F(FlockTestGroup, move_assignment_moves_resources) {
    libsarus::Flock original{fileToLock, libsarus::Flock::Type::writeLock};
    {
        libsarus::Flock moveAssigned;
        moveAssigned = std::move(original);
        CHECK_THROWS(libsarus::Error, libsarus::Flock(fileToLock, libsarus::Flock::Type::writeLock, 10_ms));
    }
    // check that lock can be acquired (move-assigned lock went out of scope)
    CHECK(lockAcquisitionDoesntThrow(fileToLock, libsarus::Flock::Type::writeLock));
}

TEST_F(FlockTestGroup, write_fails_if_resource_is_in_use) {
    {
        libsarus::Flock lock{fileToLock, libsarus::Flock::Type::writeLock};
        CHECK_THROWS(libsarus::Error, libsarus::Flock(fileToLock, libsarus::Flock::Type::writeLock, 10_ms));
    }
    {
        libsarus::Flock lock{fileToLock, libsarus::Flock::Type::readLock};
        CHECK_THROWS(libsarus::Error, libsarus::Flock(fileToLock, libsarus::Flock::Type::writeLock, 10_ms));
    }
}

TEST_F(FlockTestGroup, concurrent_read_are_allowed) {
    libsarus::Flock lock{fileToLock, libsarus::Flock::Type::readLock};
    CHECK(lockAcquisitionDoesntThrow(fileToLock, libsarus::Flock::Type::readLock));
}

TEST_F(FlockTestGroup, read_fails_if_resource_is_being_written) {
    libsarus::Flock lock{fileToLock, libsarus::Flock::Type::writeLock};
    CHECK_THROWS(libsarus::Error, libsarus::Flock(fileToLock, libsarus::Flock::Type::readLock, 10_ms));
}

TEST_F(FlockTestGroup, convert_read_to_write) {
    libsarus::Flock lock{fileToLock, libsarus::Flock::Type::readLock};
    lock.convertToType(libsarus::Flock::Type::writeLock);
    CHECK_THROWS(libsarus::Error, libsarus::Flock(fileToLock, libsarus::Flock::Type::readLock, 10_ms));
}

TEST_F(FlockTestGroup, convert_write_to_read) {
    libsarus::Flock lock{fileToLock, libsarus::Flock::Type::writeLock};
    lock.convertToType(libsarus::Flock::Type::readLock);
    CHECK(lockAcquisitionDoesntThrow(fileToLock, libsarus::Flock::Type::readLock));
}

TEST_F(FlockTestGroup, timeout_time_is_respected) {
    libsarus::Flock lock{fileToLock, libsarus::Flock::Type::writeLock};

    for (auto &timeout : {100, 500, 1000, 2000}) {
        auto start = std::chrono::system_clock::now();
        try {
            libsarus::Flock{fileToLock, libsarus::Flock::Type::writeLock, std::chrono::milliseconds{timeout}};
        } catch (const libsarus::Error &) {
        }
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        CHECK(timeout <= elapsed.count());
        CHECK(elapsed.count() <= 2 * timeout);
    }
}

static_assert(!std::is_copy_constructible<libsarus::Flock>::value, "");
static_assert(!std::is_copy_assignable<libsarus::Flock>::value, "");
static_assert(std::is_move_constructible<libsarus::Flock>::value, "");
static_assert(std::is_move_assignable<libsarus::Flock>::value, "");

}}

