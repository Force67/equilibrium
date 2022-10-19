// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "zeta_connection.h"

namespace network {
namespace {
TEST(ZetaConnectionTest, Create) {
  sockpp::inet_address none;
  ZetaConnection conn(none, nullptr);
  __debugbreak();
}
}  // namespace

}  // namespace network