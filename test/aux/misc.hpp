/*
 * Sarus
 *
 * Copyright (c) 2018-2023, ETH Zurich. All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <sys/types.h>
#include <tuple>

namespace libsarus {
namespace test {
namespace aux {
namespace misc {

std::tuple<uid_t, gid_t> getNonRootUserIds();

}
} // namespace aux
} // namespace test
} // namespace libsarus
