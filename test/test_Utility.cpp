/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <array>
#include <unistd.h>
#include <sys/fsuid.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <gtest/gtest.h>

#include "aux/misc.hpp"
#include "PathRAII.hpp"
#include "Utility.hpp"


namespace libsarus {
namespace test {

class UtilityTestSuite : public testing::Test {
protected:
};

TEST_F(UtilityTestSuite, parseEnvironmentVariables) {
    // empty environment
    {
        auto env = std::array<char*, 1>{nullptr};
        auto map = libsarus::environment::parseVariables(env.data());
        EXPECT_TRUE(map.empty());
    }
    // non-empty environment
    {
        auto var0 = std::string{"key0="};
        auto var1 = std::string{"key1=value1"};
        auto env = std::array<char*, 3>{&var0[0], &var1[0], nullptr};
        auto actualMap = libsarus::environment::parseVariables(env.data());
        auto expectedMap = std::unordered_map<std::string, std::string>{
            {"key0", ""},
            {"key1", "value1"}
        };
        EXPECT_EQ(actualMap, expectedMap);
    }
}

TEST_F(UtilityTestSuite, getEnvironmentVariable) {
    auto testKey = std::string{"SARUS_UNITTEST_GETVAR"};
    auto testValue = std::string{"dummy"};

    // test with variable unset
    EXPECT_THROW(libsarus::environment::getVariable(testKey), libsarus::Error);

    // test with variable set
    libsarus::environment::setVariable(testKey, testValue);
    auto fallbackValue = std::string("fallback");
    EXPECT_EQ(libsarus::environment::getVariable(testKey), testValue);
}

TEST_F(UtilityTestSuite, setEnvironmentVariable) {
    auto testKey = std::string{"SARUS_UNITTEST_SETVAR"};
    auto testValue = std::string{"dummy"};

    // test with variable not set
    if (unsetenv(testKey.c_str()) != 0) {
        auto message = boost::format("Error un-setting the variable used by the test: %s") % strerror(errno);
        FAIL() << message.str().c_str();
    }
    libsarus::environment::setVariable(testKey, testValue);
    char *envValue = getenv(testKey.c_str());
    if (envValue == nullptr) {
        auto message = boost::format("Error getting the test variable from the environment");
        FAIL() << message.str().c_str();
    }
    EXPECT_EQ(std::string(envValue), testValue);

    // test overwrite with variable set
    testValue = std::string{"overwrite_dummy"};
    libsarus::environment::setVariable(testKey, testValue);
    envValue = getenv(testKey.c_str());
    if (envValue == nullptr) {
        auto message = boost::format("Error getting the test variable from the environment");
        FAIL() << message.str().c_str();
    }
    EXPECT_EQ(std::string(envValue), testValue);
}

TEST_F(UtilityTestSuite, parseKeyValuePair) {
    auto pair = libsarus::string::parseKeyValuePair("key=value");
    EXPECT_EQ(pair.first, std::string("key"));
    EXPECT_EQ(pair.second, std::string("value"));

    // key only
    pair = libsarus::string::parseKeyValuePair("key_only");
    EXPECT_EQ(pair.first, std::string("key_only"));
    EXPECT_EQ(pair.second, std::string(""));

    // no value after separator
    pair = libsarus::string::parseKeyValuePair("key=");
    EXPECT_EQ(pair.first, std::string("key"));
    EXPECT_EQ(pair.second, std::string(""));

    // non-default separator
    pair = libsarus::string::parseKeyValuePair("key:value", ':');
    EXPECT_EQ(pair.first, std::string("key"));
    EXPECT_EQ(pair.second, std::string("value"));

    // empty input
    EXPECT_THROW(libsarus::string::parseKeyValuePair(""), libsarus::Error);

    // missing key
    EXPECT_THROW(libsarus::string::parseKeyValuePair("=value"), libsarus::Error);
}

TEST_F(UtilityTestSuite, switchIdentity) {
    auto testDirRAII = libsarus::PathRAII{ "./sarus-test-switchIdentity" };
    libsarus::filesystem::createFileIfNecessary(testDirRAII.getPath() / "file", 0, 0);
    boost::filesystem::permissions(testDirRAII.getPath(), boost::filesystem::owner_all);

    uid_t unprivilegedUid;
    gid_t unprivilegedGid;
    std::tie(unprivilegedUid, unprivilegedGid) = aux::misc::getNonRootUserIds();
    auto unprivilegedIdentity = libsarus::UserIdentity{unprivilegedUid, unprivilegedGid, {}};

    libsarus::process::switchIdentity(unprivilegedIdentity);

    // Check identity change
    EXPECT_EQ(geteuid(), unprivilegedIdentity.uid);
    EXPECT_EQ(getegid(), unprivilegedIdentity.gid);

    // Check it's not possible to read root-owned files or write in root-owned dirs
    EXPECT_THROW(boost::filesystem::exists(testDirRAII.getPath() / "file"), std::exception);
    EXPECT_THROW(libsarus::filesystem::createFileIfNecessary(testDirRAII.getPath() / "file_fail"), std::exception);

    auto rootIdentity = libsarus::UserIdentity{};
    libsarus::process::switchIdentity(rootIdentity);

    EXPECT_EQ(geteuid(), 0);
    EXPECT_EQ(getegid(), 0);
    EXPECT_TRUE(boost::filesystem::exists(testDirRAII.getPath() / "file"));
}

TEST_F(UtilityTestSuite, setFilesystemUid) {
    // switch to unprivileged user
    uid_t unprivilegedUid;
    gid_t unprivilegedGid;
    std::tie(unprivilegedUid, unprivilegedGid) = aux::misc::getNonRootUserIds();
    auto unprivilegedIdentity = libsarus::UserIdentity{unprivilegedUid, unprivilegedGid, {}};
    auto rootIdentity = libsarus::UserIdentity{};

    libsarus::process::setFilesystemUid(unprivilegedIdentity);

    // check identity change
    EXPECT_EQ(getuid(), rootIdentity.uid);
    EXPECT_EQ(getgid(), rootIdentity.gid);
    EXPECT_EQ(geteuid(), rootIdentity.uid);
    EXPECT_EQ(getegid(), rootIdentity.gid);
    EXPECT_EQ((uid_t)setfsuid(-1), unprivilegedIdentity.uid);

    // switch back to privileged fsuid
    libsarus::process::setFilesystemUid(rootIdentity);

    // check identity change
    EXPECT_EQ(getuid(), rootIdentity.uid);
    EXPECT_EQ(getgid(), rootIdentity.gid);
    EXPECT_EQ(geteuid(), rootIdentity.uid);
    EXPECT_EQ(getegid(), rootIdentity.gid);
    EXPECT_EQ((uid_t)setfsuid(-1), rootIdentity.uid);
}

TEST_F(UtilityTestSuite, executeCommand) {
    EXPECT_EQ(libsarus::process::executeCommand("printf stdout"), std::string{"stdout"});
    EXPECT_EQ(libsarus::process::executeCommand("bash -c 'printf stderr >&2'"), std::string{"stderr"});
    EXPECT_THROW(libsarus::process::executeCommand("false"), libsarus::Error);
    EXPECT_THROW(libsarus::process::executeCommand("command-that-doesnt-exist-xyz"), libsarus::Error);
}

TEST_F(UtilityTestSuite, makeUniquePathWithRandomSuffix) {
    auto path = boost::filesystem::path{"/tmp/file"};
    auto uniquePath = libsarus::filesystem::makeUniquePathWithRandomSuffix(path);

    auto matches = boost::cmatch{};
    auto expectedRegex = boost::regex("^/tmp/file-[A-z]{16}$");
    EXPECT_TRUE(boost::regex_match(uniquePath.string().c_str(), matches, expectedRegex));
}

TEST_F(UtilityTestSuite, createFoldersIfNecessary) {
    if (boost::filesystem::exists("/tmp/grandparent"))
        boost::filesystem::remove_all("/tmp/grandparent");

    libsarus::filesystem::createFoldersIfNecessary("/tmp/grandparent/parent/child");
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/grandparent/parent"), (std::tuple<uid_t, gid_t>{0, 0}));
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/grandparent/parent/child"), (std::tuple<uid_t, gid_t>{0, 0}));
    boost::filesystem::remove_all("/tmp/grandparent");

    libsarus::filesystem::createFoldersIfNecessary("/tmp/grandparent/parent/child", 1000, 1000);
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/grandparent/parent"), (std::tuple<uid_t, gid_t>{1000, 1000}));
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/grandparent/parent/child"), (std::tuple<uid_t, gid_t>{1000, 1000}));
    boost::filesystem::remove_all("/tmp/grandparent");
}

TEST_F(UtilityTestSuite, createFileIfNecessary) {
    if (boost::filesystem::exists("/tmp/testFile"))
        boost::filesystem::remove_all("/tmp/testFile");

    libsarus::filesystem::createFileIfNecessary("/tmp/testFile");
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/testFile"), (std::tuple<uid_t, gid_t>{0, 0}));
    boost::filesystem::remove_all("/tmp/testFile");

    libsarus::filesystem::createFileIfNecessary("/tmp/testFile", 1000, 1000);
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/testFile"), (std::tuple<uid_t, gid_t>{1000, 1000}));
    boost::filesystem::remove_all("/tmp/testFile");
}

TEST_F(UtilityTestSuite, copyFile) {
    auto testDirRAII = libsarus::PathRAII{ "./sarus-test-copyFile" };
    const auto& testDir = testDirRAII.getPath();
    libsarus::filesystem::createFileIfNecessary(testDir / "src");

    // implicit owner
    libsarus::filesystem::copyFile(testDir / "src", testDir / "dst");
    EXPECT_EQ(libsarus::filesystem::getOwner(testDir / "dst"), (std::tuple<uid_t, gid_t>{0, 0}));

    // explicit owner + overwrite existing file
    libsarus::filesystem::copyFile(testDir / "src", testDir / "dst", 1000, 1000);
    EXPECT_EQ(libsarus::filesystem::getOwner(testDir / "dst"), (std::tuple<uid_t, gid_t>{1000, 1000}));

    // explicit owner + non-existing directory
    libsarus::filesystem::copyFile(testDir / "src", testDir / "non-existing-folder/dst", 1000, 1000);
    EXPECT_EQ(libsarus::filesystem::getOwner(testDir / "non-existing-folder"), (std::tuple<uid_t, gid_t>{1000, 1000}));
    EXPECT_EQ(libsarus::filesystem::getOwner(testDir / "non-existing-folder/dst"), (std::tuple<uid_t, gid_t>{1000, 1000}));

    boost::filesystem::remove_all(testDir);
}

TEST_F(UtilityTestSuite, copyFolder) {
    if (boost::filesystem::exists("/tmp/src-folder"))
        boost::filesystem::remove_all("/tmp/src-folder");
    if (boost::filesystem::exists("/tmp/dst-folder"))
        boost::filesystem::remove_all("/tmp/dst-folder");

    libsarus::filesystem::createFoldersIfNecessary("/tmp/src-folder/subfolder");
    libsarus::filesystem::createFileIfNecessary("/tmp/src-folder/file0");
    libsarus::filesystem::createFileIfNecessary("/tmp/src-folder/subfolder/file1");

    libsarus::filesystem::copyFolder("/tmp/src-folder", "/tmp/dst-folder");
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/dst-folder/file0"), (std::tuple<uid_t, gid_t>{0, 0}));
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/dst-folder/subfolder/file1"), (std::tuple<uid_t, gid_t>{0, 0}));
    boost::filesystem::remove_all("/tmp/dst-folder");

    libsarus::filesystem::copyFolder("/tmp/src-folder", "/tmp/dst-folder", 1000, 1000);
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/dst-folder/file0"), (std::tuple<uid_t, gid_t>{1000, 1000}));
    EXPECT_EQ(libsarus::filesystem::getOwner("/tmp/dst-folder/subfolder/file1"), (std::tuple<uid_t, gid_t>{1000, 1000}));
    boost::filesystem::remove_all("/tmp/dst-folder");
    boost::filesystem::remove_all("/tmp/src-folder");
}

TEST_F(UtilityTestSuite, countFilesInDirectory) {
    // nominal usage
    {
        auto testDir = boost::filesystem::path("/tmp/file-count-test");
        libsarus::filesystem::createFoldersIfNecessary(testDir);
        libsarus::filesystem::createFileIfNecessary(testDir / "file1");
        libsarus::filesystem::createFileIfNecessary(testDir / "file2");
        libsarus::filesystem::createFileIfNecessary(testDir / "file3");
        libsarus::filesystem::createFileIfNecessary(testDir / "file4");
        EXPECT_EQ(libsarus::filesystem::countFilesInDirectory(testDir), 4);

        boost::filesystem::remove(testDir / "file1");
        boost::filesystem::remove(testDir / "file4");
        EXPECT_EQ(libsarus::filesystem::countFilesInDirectory(testDir), 2);

        boost::filesystem::remove_all(testDir);
    }
    // non-existing directory
    {
        EXPECT_THROW(libsarus::filesystem::countFilesInDirectory("/tmp/" + libsarus::string::generateRandom(16)), libsarus::Error);
    }
    // non-directory argument
    {
        auto testFile = libsarus::PathRAII{"/tmp/file-count-test.txt"};
        libsarus::filesystem::createFileIfNecessary(testFile.getPath());
        EXPECT_THROW(libsarus::filesystem::countFilesInDirectory(testFile.getPath()), libsarus::Error);
    }
}

TEST_F(UtilityTestSuite, parseMap) {
    // empty list
    {
        auto map = libsarus::string::parseMap("");
        EXPECT_TRUE(map.empty());
    }
    // one key-value pair
    {
        auto list = "key0=value0";
        auto map = libsarus::string::parseMap(list);
        EXPECT_EQ(map.size(), 1);
        EXPECT_EQ(map["key0"], std::string{"value0"});
    }
    // two key-value pairs
    {
        auto list = "key0=value0,key1=value1";
        auto map = libsarus::string::parseMap(list);
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map["key0"], std::string{"value0"});
        EXPECT_EQ(map["key1"], std::string{"value1"});
    }
    // key only (no value associated)
    {
        auto list = "key_only";
        auto map = libsarus::string::parseMap(list);
        EXPECT_EQ(map.size(), 1);
        EXPECT_EQ(map["key_only"], "");
    }
    {
        auto list = "key_only_at_begin,key=value";
        auto map = libsarus::string::parseMap(list);
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map["key_only_at_begin"], "");
        EXPECT_EQ(map["key"], "value");
    }
    {
        auto list = "key=value,key_only_at_end";
        auto map = libsarus::string::parseMap(list);
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map["key"], "value");
        EXPECT_EQ(map["key_only_at_end"], "");
    }
    {
        auto list = "key_only0,key_only1";
        auto map = libsarus::string::parseMap(list);
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map["key_only0"], "");
        EXPECT_EQ(map["key_only1"], "");
    }
    // missing key error
    {
        auto list = ",key=value";
        EXPECT_THROW(libsarus::string::parseMap(list), libsarus::Error);
    }
    {
        auto list = "key0=value0,,key1=value1";
        EXPECT_THROW(libsarus::string::parseMap(list), libsarus::Error);
    }
    {
        auto list = "key0=value0,";
        EXPECT_THROW(libsarus::string::parseMap(list), libsarus::Error);
    }
    // repeated key error
    {
        auto list = "key0=value0,key0=value1";
        EXPECT_THROW(libsarus::string::parseMap(list), libsarus::Error);
    }
    // too many values error, a.k.a. repeated kv separator
    {
        auto list = "key0=value0=value1";
        EXPECT_THROW(libsarus::string::parseMap(list), libsarus::Error);
    }
}

TEST_F(UtilityTestSuite, realpathWithinRootfs) {
    auto path = libsarus::PathRAII{libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/sarus-rootfs")};
    const auto& rootfs = path.getPath();

    libsarus::filesystem::createFoldersIfNecessary(rootfs / "dir0/dir1");
    libsarus::filesystem::createFoldersIfNecessary(rootfs / "dirX");
    libsarus::filesystem::createFileIfNecessary(rootfs / "dir0/dir1/file");

    // folder
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1"), "/dir0/dir1");

    // file
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/file"), "/dir0/dir1/file");

    // relative symlink
    EXPECT_EQ(symlink("../../dir0/dir1", (rootfs / "dir0/dir1/link_relative").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_relative"), "/dir0/dir1");

    // relative symlink that spills (out of rootfs)
    EXPECT_EQ(symlink("../../../../dir0/dir1", (rootfs / "dir0/dir1/link_relative_that_spills").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_relative_that_spills"), "/dir0/dir1");

    // relative symlink recursive
    EXPECT_EQ(symlink("../../dir0/dir1/link_relative/dir2/dir3", (rootfs / "dir0/dir1/link_relative_recursive").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_relative_recursive"), "/dir0/dir1/dir2/dir3");

    // relative symlink recursive that spills (out of rootfs)
    EXPECT_EQ(symlink("../../../dir0/dir1/link_relative_that_spills/dir2/dir3", (rootfs / "dir0/dir1/link_relative_recursive_that_spills").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_relative_recursive_that_spills"), "/dir0/dir1/dir2/dir3");

    // absolute symlink
    EXPECT_EQ(symlink("/dir0/dir1", (rootfs / "dir0/dir1/link_absolute").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_absolute"), "/dir0/dir1");

    // absolute symlink that spills (out of rootfs)
    EXPECT_EQ(symlink("/dir0/dir1/../../../../dir0/dir1", (rootfs / "dir0/dir1/link_absolute_that_spills").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_absolute_that_spills"), "/dir0/dir1");

    // absolute symlink recursive
    EXPECT_EQ(symlink("/dir0/dir1/link_absolute/dir2/dir3", (rootfs / "dir0/dir1/link_absolute_recursive").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_absolute_recursive"), "/dir0/dir1/dir2/dir3");

    // absolute symlink recursive that spills (out of rootfs)
    EXPECT_EQ(symlink("/dir0/dir1/link_absolute_that_spills/dir2/dir3", (rootfs / "dir0/dir1/link_absolute_recursive_that_spills").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dir0/dir1/link_absolute_recursive_that_spills"), "/dir0/dir1/dir2/dir3");

    // absolute symlink sharing no part of the path with the target
    EXPECT_EQ(symlink("/dir0/dir1", (rootfs / "dirX/link_absolute_with_no_common_path").string().c_str()), 0);
    EXPECT_EQ(libsarus::filesystem::realpathWithinRootfs(rootfs, "/dirX/link_absolute_with_no_common_path"), "/dir0/dir1");
}

TEST_F(UtilityTestSuite, getSharedLibLinkerName) {
    EXPECT_EQ(libsarus::sharedlibs::getLinkerName("file.so"), "file.so");
    EXPECT_EQ(libsarus::sharedlibs::getLinkerName("file.so.1"), "file.so");
    EXPECT_EQ(libsarus::sharedlibs::getLinkerName("file.so.1.0"), "file.so");
    EXPECT_EQ(libsarus::sharedlibs::getLinkerName("file.so.1.0.0"), "file.so");

    EXPECT_THROW(libsarus::sharedlibs::getLinkerName("not-a-shared-lib"), libsarus::Error);
    EXPECT_THROW(libsarus::sharedlibs::getLinkerName("not-a-shared-lib.soa"), libsarus::Error);
}

TEST_F(UtilityTestSuite, isSharedLib) {
    EXPECT_EQ(libsarus::filesystem::isSharedLib("/dir/libc.so"), true);
    EXPECT_EQ(libsarus::filesystem::isSharedLib("libc.so"), true);
    EXPECT_EQ(libsarus::filesystem::isSharedLib("libc.so.1"), true);
    EXPECT_EQ(libsarus::filesystem::isSharedLib("libc.so.1.2"), true);

    EXPECT_EQ(libsarus::filesystem::isSharedLib("libc"), false);
    EXPECT_EQ(libsarus::filesystem::isSharedLib("libc.s"), false);
    EXPECT_EQ(libsarus::filesystem::isSharedLib("ld.so.conf"), false);
    EXPECT_EQ(libsarus::filesystem::isSharedLib("ld.so.cache"), false);
}

TEST_F(UtilityTestSuite, parseSharedLibAbi) {
    EXPECT_THROW(libsarus::sharedlibs::parseAbi("invalid"), libsarus::Error);
    EXPECT_EQ(libsarus::sharedlibs::parseAbi("libc.so"), (std::vector<std::string>{}));
    EXPECT_EQ(libsarus::sharedlibs::parseAbi("libc.so.1"), (std::vector<std::string>{"1"}));
    EXPECT_EQ(libsarus::sharedlibs::parseAbi("libc.so.1.2"), (std::vector<std::string>{"1", "2"}));
    EXPECT_EQ(libsarus::sharedlibs::parseAbi("libc.so.1.2.3"), (std::vector<std::string>{"1", "2", "3"}));
    EXPECT_EQ(libsarus::sharedlibs::parseAbi("libc.so.1.2.3rc1"), (std::vector<std::string>{"1", "2", "3rc1"}));

    EXPECT_EQ(libsarus::sharedlibs::parseAbi("libfoo.so.0"), (std::vector<std::string>{"0"}));
}

TEST_F(UtilityTestSuite, resolveSharedLibAbi) {
    auto testDirRaii = libsarus::PathRAII{
        libsarus::filesystem::makeUniquePathWithRandomSuffix("/tmp/sarus-test-utility-resolveSharedLibAbi")
    };
    const auto& testDir = testDirRaii.getPath();

    // invalid library filename
    libsarus::filesystem::createFileIfNecessary(testDir / "invalid");
    EXPECT_THROW(libsarus::sharedlibs::resolveAbi(testDir / "invalid"), libsarus::Error);

    // libtest.so
    libsarus::filesystem::createFileIfNecessary(testDir / "libtest.so");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "libtest.so"), std::vector<std::string>{});

    // libtest.so.1
    libsarus::filesystem::createFileIfNecessary(testDir / "libtest.so.1");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "libtest.so.1"), std::vector<std::string>{"1"});

    // libtest_symlink.so.1 -> libtest_symlink.so.1.2
    libsarus::filesystem::createFileIfNecessary(testDir / "libtest_symlink.so.1.2");
    boost::filesystem::create_symlink(testDir / "libtest_symlink.so.1.2", testDir / "libtest_symlink.so.1");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "libtest_symlink.so.1"), (std::vector<std::string>{"1", "2"}));

    // libtest_symlink.so.1.2.3 -> libtest_symlink.so.1.2
    boost::filesystem::create_symlink(testDir / "libtest_symlink.so.1.2", testDir / "libtest_symlink.so.1.2.3");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "libtest_symlink.so.1.2.3"), (std::vector<std::string>{"1", "2", "3"}));

    // libtest_symlink.so -> libtest_symlink.so.1.2.3 -> libtest_symlink.so.1.2
    boost::filesystem::create_symlink(testDir / "libtest_symlink.so.1.2.3", testDir / "libtest_symlink.so");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "libtest_symlink.so"), (std::vector<std::string>{"1", "2", "3"}));

    // subdir/libtest_symlink.so -> ../libtest_symlink.so.1.2.3 -> libtest_symlink.so.1.2
    libsarus::filesystem::createFoldersIfNecessary(testDir / "subdir");
    boost::filesystem::create_symlink("../libtest_symlink.so.1.2.3", testDir / "subdir/libtest_symlink.so");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "subdir/libtest_symlink.so"), (std::vector<std::string>{"1", "2", "3"}));

    // /libtest_symlink_within_rootdir.so -> /subdir/libtest_symlink_within_rootdir.so.1 -> ../libtest_symlink_within_rootdir.so.1.2
    boost::filesystem::create_symlink("/subdir/libtest_symlink_within_rootdir.so.1", testDir / "libtest_symlink_within_rootdir.so");
    boost::filesystem::create_symlink("../libtest_symlink_within_rootdir.so.1.2", testDir / "/subdir/libtest_symlink_within_rootdir.so.1");
    libsarus::filesystem::createFileIfNecessary(testDir / "libtest_symlink_within_rootdir.so.1.2");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi("/libtest_symlink_within_rootdir.so", testDir), (std::vector<std::string>{"1", "2"}));

    // Some vendors have symlinks with incompatible major versions,
    // like libvdpau_nvidia.so.1 -> libvdpau_nvidia.so.440.33.01.
    // For these cases, we trust the vendor and resolve the Lib Abi to that of the symlink.
    // Note here we use libtest.so.1 as the "original lib file" and create a symlink to it.
    boost::filesystem::create_symlink(testDir / "libtest.so.1", testDir / "libtest.so.234.56");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "libtest.so.234.56"), (std::vector<std::string>{"234", "56"}));

    boost::filesystem::create_symlink("../libtest.so.1.2", testDir / "subdir" / "libtest.so.234.56");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "subdir" / "libtest.so.234.56"), (std::vector<std::string>{"234", "56"}));

    boost::filesystem::create_symlink("../libtest.so.1.2", testDir / "subdir" / "libtest.so.234");
    EXPECT_EQ(libsarus::sharedlibs::resolveAbi(testDir / "subdir" / "libtest.so.234"), (std::vector<std::string>{"234"}));
}

TEST_F(UtilityTestSuite, getSharedLibSoname) {
    auto dummyLibsDir = boost::filesystem::path{__FILE__}
        .parent_path() / "dummy_libs";
    EXPECT_EQ(libsarus::sharedlibs::getSoname(dummyLibsDir / "libc.so.6-host", "readelf"), std::string("libc.so.6"));
    EXPECT_EQ(libsarus::sharedlibs::getSoname(dummyLibsDir / "ld-linux-x86-64.so.2-host", "readelf"), std::string("ld-linux-x86-64.so.2"));
    EXPECT_THROW(libsarus::sharedlibs::getSoname(dummyLibsDir / "lib_dummy_0.so", "readelf"), libsarus::Error);
}

TEST_F(UtilityTestSuite, isLibc) {
    // libc
    EXPECT_TRUE(libsarus::filesystem::isLibc("libc.so"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("libc.so.6"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("libc-2.29.so"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("/libc.so"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("../libc.so"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("dir/libc.so"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("dir/dir/libc.so"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("/root/libc.so"));
    EXPECT_TRUE(libsarus::filesystem::isLibc("/root/dir/libc.so"));

    // not libc
    EXPECT_FALSE(libsarus::filesystem::isLibc("libcl.so"));
    EXPECT_FALSE(libsarus::filesystem::isLibc("libc_bogus.so"));
}

TEST_F(UtilityTestSuite, is64bitSharedLib) {
    auto dummyLibsDir = boost::filesystem::path{__FILE__}
        .parent_path() / "dummy_libs";
    EXPECT_TRUE(libsarus::sharedlibs::is64bitSharedLib(dummyLibsDir / "libc.so.6-host", "readelf"));
    EXPECT_TRUE(libsarus::sharedlibs::is64bitSharedLib(dummyLibsDir / "ld-linux-x86-64.so.2-host", "readelf"));
    EXPECT_FALSE(libsarus::sharedlibs::is64bitSharedLib(dummyLibsDir / "libc.so.6-32bit-container", "readelf"));
}

TEST_F(UtilityTestSuite, serializeJSON) {
    namespace rj = rapidjson;
    auto json = rj::Document{rj::kObjectType};
    auto& allocator = json.GetAllocator();

    json.AddMember("string", rj::Value{"stringValue", allocator}, allocator);
    json.AddMember("int", rj::Value{11}, allocator);
    json.AddMember("array", rj::Value{rj::kArrayType}, allocator);
    json["array"].PushBack(rj::Value{0}, allocator);
    json["array"].PushBack(rj::Value{1}, allocator);
    json["array"].PushBack(rj::Value{2}, allocator);

    auto actual = libsarus::json::serialize(json);
    auto expected = std::string{"{\"string\":\"stringValue\",\"int\":11,\"array\":[0,1,2]}"};

    EXPECT_EQ(libsarus::string::removeWhitespaces(actual), expected);
}

TEST_F(UtilityTestSuite, setCpuAffinity_invalid_argument) {
    EXPECT_THROW(libsarus::process::setCpuAffinity({}), libsarus::Error); // no CPUs
}

TEST_F(UtilityTestSuite, getCpuAffinity_setCpuAffinity) {
    auto initialCpus = libsarus::process::getCpuAffinity();

    if(initialCpus.size() <= 1) {
        std::cerr << "Skipping CPU affinity unit test. Not enough CPUs available" << std::endl;
        return;
    }

    // set new affinity (removing one CPU)
    auto newCpus = initialCpus;
    newCpus.pop_back();
    libsarus::process::setCpuAffinity(newCpus);

    // check
    EXPECT_EQ(libsarus::process::getCpuAffinity(), newCpus);

    // restore initial affinity
    libsarus::process::setCpuAffinity(initialCpus);
}

}}

