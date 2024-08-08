/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <fstream>
#include <streambuf>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

#include "PasswdDB.hpp"
#include "PathRAII.hpp"


namespace libsarus {
namespace test {

class PasswdDBTestGroup : public testing::Test {
protected:
    PasswdDB passwd{};

    PasswdDBTestGroup() {
        // create entry
        auto entry0 = PasswdDB::Entry{
            "loginName0",
            "x",
            1000,
            1001,
            "UserNameOrCommentField0",
            "/home/dir0",
            boost::filesystem::path{"/optional/UserCommandInterpreter0"}
        };
        auto entry1 = PasswdDB::Entry {
            "loginName1",
            "y",
            2000,
            2001,
            "UserNameOrCommentField1",
            "/home/dir1",
            {}
        };
        passwd.getEntries() = {entry0, entry1};
    }
};

TEST_F(PasswdDBTestGroup, testRead) {
    // create file
    auto path = libsarus::PathRAII{boost::filesystem::path{"/tmp/test-passwd-file"}};
    const auto& file = path.getPath();
    std::ofstream of{file.c_str()};
    of  << "loginName0:x:1000:1001:UserNameOrCommentField0:/home/dir0"
        << std::endl
        << "loginName1:encryptedPass1:4294967294:4294967294:UserNameOrCommentField1:/home/dir1:/optional/UserCommandInterpreter1"
        << std::endl
        << "loginName2:x:1000:1001:UserNameOrCommentField2:/home/dir2:"
        << std::endl;

    // read from file
    passwd = PasswdDB{file};
    const auto& entries = passwd.getEntries();

    EXPECT_EQ(entries.size(), 3);

    EXPECT_EQ(entries[0].loginName, "loginName0");
    EXPECT_EQ(entries[0].encryptedPassword, "x");
    EXPECT_EQ(entries[0].uid, 1000);
    EXPECT_EQ(entries[0].gid, 1001);
    EXPECT_EQ(entries[0].userNameOrCommentField, "UserNameOrCommentField0");
    EXPECT_EQ(entries[0].userHomeDirectory, "/home/dir0");
    EXPECT_FALSE(entries[0].userCommandInterpreter);

    EXPECT_EQ(entries[1].loginName, "loginName1");
    EXPECT_EQ(entries[1].encryptedPassword, "encryptedPass1");
    EXPECT_EQ(entries[1].uid, 4294967294UL);
    EXPECT_EQ(entries[1].gid, 4294967294UL);
    EXPECT_EQ(entries[1].userNameOrCommentField, "UserNameOrCommentField1");
    EXPECT_EQ(entries[1].userHomeDirectory, "/home/dir1");
    EXPECT_EQ(*entries[1].userCommandInterpreter, "/optional/UserCommandInterpreter1");

    EXPECT_EQ(entries[2].loginName, "loginName2");
    EXPECT_EQ(entries[2].encryptedPassword, "x");
    EXPECT_EQ(entries[2].uid, 1000);
    EXPECT_EQ(entries[2].gid, 1001);
    EXPECT_EQ(entries[2].userNameOrCommentField, "UserNameOrCommentField2");
    EXPECT_EQ(entries[2].userHomeDirectory, "/home/dir2");
    EXPECT_FALSE(entries[2].userCommandInterpreter);
}

TEST_F(PasswdDBTestGroup, testWrite) {
    auto path = libsarus::PathRAII{boost::filesystem::path{"/tmp/test-passwd-file"}};
    const auto& file = path.getPath();

    // write to file
    passwd.write(file);

    // check file contents
    std::ifstream is(file.c_str());
    auto data = std::string(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
    auto expectedData = std::string{"loginName0:x:1000:1001:UserNameOrCommentField0:/home/dir0:/optional/UserCommandInterpreter0\n"
                                    "loginName1:y:2000:2001:UserNameOrCommentField1:/home/dir1:\n"};
    EXPECT_EQ(data, expectedData);
}

TEST_F(PasswdDBTestGroup, testGetUsername) {
    EXPECT_EQ(passwd.getUsername(1000), std::string{"loginName0"});
    EXPECT_EQ(passwd.getUsername(2000), std::string{"loginName1"});
}

TEST_F(PasswdDBTestGroup, testGetHomeDirectory) {
    EXPECT_EQ(passwd.getHomeDirectory(1000), boost::filesystem::path{"/home/dir0"});
    EXPECT_EQ(passwd.getHomeDirectory(2000), boost::filesystem::path{"/home/dir1"});
}

}}

