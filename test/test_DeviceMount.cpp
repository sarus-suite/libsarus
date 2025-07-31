/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string>

#include <sys/mount.h>
#include <sys/sysmacros.h>

#include <gtest/gtest.h>

#include "libsarus/DeviceAccess.hpp"
#include "libsarus/DeviceMount.hpp"
#include "libsarus/PathRAII.hpp"
#include "libsarus/Utility.hpp"
#include "libsarus/test/aux/filesystem.hpp"

namespace libsarus {
namespace test {

class DeviceMountTest : public testing::Test {
  protected:
};

TEST_F(DeviceMountTest, constructor) {
    auto testDir =
        libsarus::PathRAII(libsarus::filesystem::makeUniquePathWithRandomSuffix(
            boost::filesystem::current_path() /
            "deviceMount-test-constructor"));
    libsarus::filesystem::createFoldersIfNecessary(testDir.getPath());

    auto bundleDirRAII =
        libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix(
            boost::filesystem::absolute("test-bundle-dir"))};
    const auto &rootfsDir = bundleDirRAII.getPath() / "rootfs";
    libsarus::UserIdentity userIdentity;

    size_t mount_flags = 0;
    auto devAccess = libsarus::DeviceAccess("rwm");

    // regular usage
    {
        auto testDeviceFile = testDir.getPath() / "testDevice";
        auto majorID = 511u;
        auto minorID = 511u;
        aux::filesystem::createCharacterDeviceFile(testDeviceFile, majorID,
                                                   minorID);
        auto mountObject =
            libsarus::Mount{testDeviceFile, testDeviceFile, mount_flags,
                            rootfsDir, userIdentity};

        DeviceMount(std::move(mountObject), devAccess);
    }
    // source path is not a device file
    {
        auto noDeviceFile = testDir.getPath() / "notADevice";
        libsarus::filesystem::createFileIfNecessary(noDeviceFile);
        auto mountObject = libsarus::Mount{
            noDeviceFile, noDeviceFile, mount_flags, rootfsDir, userIdentity};

        EXPECT_THROW(DeviceMount(std::move(mountObject), devAccess),
                     libsarus::Error);
    }
}

TEST_F(DeviceMountTest, getters) {
    auto testDir =
        libsarus::PathRAII(libsarus::filesystem::makeUniquePathWithRandomSuffix(
            boost::filesystem::current_path() / "deviceMount-test-getters"));
    libsarus::filesystem::createFoldersIfNecessary(testDir.getPath());

    auto bundleDirRAII =
        libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix(
            boost::filesystem::absolute("test-bundle-dir"))};
    const auto &rootfsDir = bundleDirRAII.getPath() / "rootfs";
    libsarus::UserIdentity userIdentity;

    size_t mount_flags = 0;

    {
        // Create source file as a character device file with 666 file mode
        auto testDeviceFile = testDir.getPath() / "sarusTestDevice0";
        auto majorID = 511u;
        auto minorID = 511u;
        aux::filesystem::createCharacterDeviceFile(testDeviceFile, majorID,
                                                   minorID);

        auto mountObject =
            libsarus::Mount{testDeviceFile, testDeviceFile, mount_flags,
                            rootfsDir, userIdentity};
        auto devAccess = libsarus::DeviceAccess("rwm");

        auto devMount = DeviceMount(std::move(mountObject), devAccess);
        EXPECT_EQ(devMount.getType(), 'c');
        EXPECT_EQ(devMount.getMajorID(), majorID);
        EXPECT_EQ(devMount.getMinorID(), minorID);
        EXPECT_EQ(devMount.getAccess().string(), std::string{"rwm"});

        boost::filesystem::remove(testDeviceFile);
    }
    {
        auto testDeviceFile = testDir.getPath() / "sarusTestDevice1";
        auto majorID = 477u;
        auto minorID = 488u;
        aux::filesystem::createBlockDeviceFile(testDeviceFile, majorID,
                                               minorID);

        auto mountObject =
            libsarus::Mount{testDeviceFile, testDeviceFile, mount_flags,
                            rootfsDir, userIdentity};
        auto devAccess = libsarus::DeviceAccess("rw");

        auto devMount = DeviceMount(std::move(mountObject), devAccess);
        EXPECT_EQ(devMount.getType(), 'b');
        EXPECT_EQ(devMount.getMajorID(), majorID);
        EXPECT_EQ(devMount.getMinorID(), minorID);
        EXPECT_EQ(devMount.getAccess().string(), std::string{"rw"});

        boost::filesystem::remove(testDeviceFile);
    }
}

TEST_F(DeviceMountTest, performMount) {
    auto testDir =
        libsarus::PathRAII(libsarus::filesystem::makeUniquePathWithRandomSuffix(
            boost::filesystem::current_path() /
            "deviceMount-test-performMount"));
    libsarus::filesystem::createFoldersIfNecessary(testDir.getPath());

    auto bundleDirRAII =
        libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix(
            boost::filesystem::absolute("test-bundle-dir"))};
    const auto &bundleDir = bundleDirRAII.getPath();
    auto rootfsDir = bundleDir / "rootfs";
    libsarus::UserIdentity userIdentity;

    libsarus::filesystem::createFoldersIfNecessary(rootfsDir);

    auto sourceFile = testDir.getPath() / "sarusTestDevice0";
    auto destinationFile = boost::filesystem::path{"/dev/sarusTestDevice0"};

    auto majorID = 511u;
    auto minorID = 511u;
    aux::filesystem::createCharacterDeviceFile(sourceFile, majorID, minorID);

    size_t mount_flags = 0;
    auto mountObject = libsarus::Mount{sourceFile, destinationFile, mount_flags,
                                       rootfsDir, userIdentity};
    auto devAccess = libsarus::DeviceAccess("rwm");

    // perform the mount
    libsarus::DeviceMount{std::move(mountObject), devAccess}.performMount();
    EXPECT_TRUE(aux::filesystem::isSameBindMountedFile(
        sourceFile, rootfsDir / destinationFile));
    EXPECT_EQ(libsarus::filesystem::getDeviceID(rootfsDir / destinationFile),
              makedev(majorID, minorID));
    EXPECT_EQ(libsarus::filesystem::getDeviceType(rootfsDir / destinationFile),
              'c');

    // cleanup
    EXPECT_EQ(umount((rootfsDir / destinationFile).c_str()), 0);
    boost::filesystem::remove(sourceFile);
}

}  // namespace test
}  // namespace libsarus
