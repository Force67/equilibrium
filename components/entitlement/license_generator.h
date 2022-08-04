// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Licensing authority: Issue.
#pragma once

#include <entitlement/license.h>
#include <base/strings/string_ref.h>

namespace entitlement {

struct LicenseInfo {
  u64 expiry_date;
  u64 start_date;
  base::String licensee_name;
  // Optional payload
  base::String payload;
};

// https
//   :  //
//   github.com/chromium/chromium/blob/78a418673dacb3ed95482a33ff593af0ea9da0f6/ash/components/arc/mojom/oemcrypto.mojom#L152

// returns the license encoded in base64.
base::String IssueLicense(const LicenseInfo& license_info,
                          const base::StringRef master_key,
                          base::String& secret);
}  // namespace entitlement