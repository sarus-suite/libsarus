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

#include <gtest/gtest.h>

#include "aux/filesystem.hpp"
#include "PathRAII.hpp"
#include "Utility.hpp"


namespace libsarus {
namespace test {

class MountUtilitiesTestGroup : public testing::Test {
protected:
};

TEST_F(MountUtilitiesTestGroup, get_validated_mount_source_test) {
    std::string mount_point("./MUMountPoint");
    std::string source_dir_1("./mount_utilities_source_1");
    libsarus::PathRAII source_dir_2RAII("./mount_utilities_source_2");
    std::string source_dir_2 = source_dir_2RAII.getPath().string();

    // Test invalid input arguments
    EXPECT_THROW(libsarus::mount::getValidatedMountSource(""), libsarus::Error);

    // Test non-existing directory
    EXPECT_THROW(libsarus::mount::getValidatedMountSource(source_dir_1), libsarus::Error);

    // Test existing directory
    libsarus::filesystem::createFoldersIfNecessary(source_dir_2);
    auto* expected = realpath(source_dir_2.c_str(), NULL);
    EXPECT_EQ(libsarus::mount::getValidatedMountSource(source_dir_2), boost::filesystem::path(expected));

    // Cleanup
    free(expected);
    boost::filesystem::remove_all(source_dir_2);
}

TEST_F(MountUtilitiesTestGroup, get_validated_mount_destination_test) {
    auto bundleDirRAII = libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix(boost::filesystem::absolute("test-bundle-dir"))}; 
    const auto& bundleDir = bundleDirRAII.getPath();
    auto rootfsDir = bundleDir / "rootfs";
    libsarus::filesystem::createFoldersIfNecessary(bundleDir / "overlay/rootfs-lower");

    // Test invalid input arguments
    EXPECT_THROW(libsarus::mount::getValidatedMountDestination("", rootfsDir), libsarus::Error);

    // Test mount on other device
    auto otherDeviceDir = boost::filesystem::path{"/otherDevice"};
    libsarus::filesystem::createFoldersIfNecessary(rootfsDir / otherDeviceDir);
    auto imageSquashfs = boost::filesystem::path{__FILE__}.parent_path() / "test_image.squashfs";
    libsarus::mount::loopMountSquashfs(imageSquashfs, rootfsDir / otherDeviceDir);
    EXPECT_THROW(libsarus::mount::getValidatedMountDestination(otherDeviceDir, rootfsDir), libsarus::Error);
    EXPECT_EQ(umount((rootfsDir / otherDeviceDir).c_str()), 0);

    // Test non-existing mount point
    auto nonExistingDir = boost::filesystem::path{"/nonExistingMountPoint"};
    auto expected = rootfsDir / nonExistingDir;
    EXPECT_EQ(libsarus::mount::getValidatedMountDestination(nonExistingDir, rootfsDir), expected);

    // Test existing mount point
    auto existingDir = boost::filesystem::path{"/file_in_squashfs_image"};
    expected = rootfsDir / existingDir;
    libsarus::filesystem::createFoldersIfNecessary(expected);
    EXPECT_EQ(libsarus::mount::getValidatedMountDestination(existingDir, rootfsDir), expected);
}

TEST_F(MountUtilitiesTestGroup, bindMount) {
    auto tempDirRAII = libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/sarus-test-common-bindmount")};
    const auto& tempDir = tempDirRAII.getPath();
    auto fromDir = tempDir / "from";
    auto toDir = tempDir / "to";

    libsarus::filesystem::createFoldersIfNecessary(fromDir);
    libsarus::filesystem::createFoldersIfNecessary(toDir);
    libsarus::filesystem::createFileIfNecessary(fromDir / "file");

    libsarus::mount::bindMount(fromDir, toDir);

    // check that "file" is in the mounted directory
    EXPECT_TRUE(boost::filesystem::exists(toDir / "file"));

    // check that mounted directory is writable
    libsarus::filesystem::createFileIfNecessary(toDir / "file-successfull-write-attempt");

    // cleanup
    EXPECT_EQ(umount(toDir.c_str()), 0);
}

TEST_F(MountUtilitiesTestGroup, bindMountReadOnly) {
    auto tempDirRAII = libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/sarus-test-common-bindmount")};
    const auto& tempDir = tempDirRAII.getPath();
    auto fromDir = tempDir / "from";
    auto toDir = tempDir / "to";

    libsarus::filesystem::createFoldersIfNecessary(fromDir);
    libsarus::filesystem::createFoldersIfNecessary(toDir);
    libsarus::filesystem::createFileIfNecessary(fromDir / "file");

    libsarus::mount::bindMount(fromDir, toDir, MS_RDONLY);

    // check that "file" is in the mounted directory
    EXPECT_TRUE(boost::filesystem::exists(toDir / "file"));

    // check that mounted directory is read-only
    EXPECT_THROW(libsarus::filesystem::createFileIfNecessary(toDir / "file-failed-write-attempt"), libsarus::Error);

    // cleanup
    EXPECT_EQ(umount(toDir.c_str()), 0);
}

TEST_F(MountUtilitiesTestGroup, bindMountRecursive) {
    auto tempDirRAII = libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/sarus-test-common-bindmount")};
    const auto& tempDir = tempDirRAII.getPath();

    auto a = tempDir / "a";
    auto b = tempDir / "b";
    auto c = tempDir / "c";
    libsarus::filesystem::createFoldersIfNecessary(a);
    libsarus::filesystem::createFoldersIfNecessary(b);
    libsarus::filesystem::createFoldersIfNecessary(c);

    libsarus::filesystem::createFileIfNecessary(c / "d.txt");

    // check that "d.txt" is in the mounted directory
    EXPECT_FALSE(boost::filesystem::exists(b / "d.txt"));
    libsarus::mount::bindMount(c, b);
    EXPECT_TRUE(boost::filesystem::exists(b / "d.txt"));

    // check that mounts are recursive by default
    EXPECT_FALSE(boost::filesystem::exists(a / "d.txt"));
    libsarus::mount::bindMount(b, a);
    EXPECT_TRUE(boost::filesystem::exists(a / "d.txt"));

    // cleanup
    EXPECT_EQ(umount(b.c_str()), 0);
    EXPECT_EQ(umount(a.c_str()), 0);
}

TEST_F(MountUtilitiesTestGroup, loopMountSquashfs) {
    auto mountPointRAII = libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/sarus-test-common-loopMountSquashfs")};
    const auto& mountPoint = mountPointRAII.getPath();
    libsarus::filesystem::createFoldersIfNecessary(mountPoint);

    auto imageSquashfs = boost::filesystem::path{__FILE__}.parent_path() / "test_image.squashfs";
    libsarus::mount::loopMountSquashfs(imageSquashfs, mountPoint);
    EXPECT_TRUE(boost::filesystem::exists(mountPoint / "file_in_squashfs_image"));

    EXPECT_EQ(umount(mountPoint.string().c_str()), 0);
}

}}

