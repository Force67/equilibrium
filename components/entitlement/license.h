// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Licensing data.
#pragma once

#include <base/arch.h>
#include <base/containers/span.h>
#include <base/strings/string_ref.h>

namespace entitlement {

struct LicenseHeader {
  enum class Type : i8 { Regular };
  u16 version;
  Type type;
  u8 reserved0;
  u16 reserved1;
  i64 issue_timestamp_;
  u64 expiry_timestamp_;
};
static_assert(sizeof(LicenseHeader) == 24);

base::String EncodeAndSignLicenseBlock(const LicenseHeader& header,
                                  const base::StringU8& licensee_name,
                                  const base::StringRefU8 optional_additional_data);
}  // namespace entitlement