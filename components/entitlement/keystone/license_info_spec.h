// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Licensing data.
#pragma once

#include <base/time/time.h>

namespace entitlement {
// tied to whom and for what
struct LicenseHeader {
  enum class Type : i8 {
    Regular,  // < Regular license owned by one single entity
    Floating  // < License for an orga
  };
  u16 version;
  Type type;
  u8 reserved0;
  u16 reserved1;
  base::time_type issue_timestamp;
  base::time_type expiry_timestamp;
};
static_assert(sizeof(LicenseHeader) == 24);
// what does the license entitle to?
struct FeatureEntilement {
  u16 version;
  u16 reserved0;
  char license_id[16];  // user readable
  u64 feature_id;       // must be in line with our global database!
  base::time_type support_expiry_timestamp;  // no more support granted, must
                                             // purchase a new license
};
static_assert(sizeof(FeatureEntilement) == 40);
}  // namespace entitlement