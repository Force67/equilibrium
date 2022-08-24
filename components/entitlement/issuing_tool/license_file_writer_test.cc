// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <license_file_writer.h>

namespace entitlement {
namespace {
TEST(LicenseFileWriterTest, EncodeLicenseInfo) {
  const LicenseHeader header{
      .version = 0,
      .type = LicenseHeader::Type::Regular,
      .reserved0 = 0,
      .reserved1 = 1,
      .issue_timestamp = 0,
      .expiry_timestamp = 0,
  };

  auto result = entitlement::EncodeLicenseBlock(
      header, u8"Vincent Van Gogh",
      u8"Neque porro quisquam est qui dolorem ipsum quia "
      "dolor sit amet, consectetur, adipisci velit...");


}
}  // namespace
}  // namespace entitlement
