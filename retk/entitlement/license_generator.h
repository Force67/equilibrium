// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Licensing authority: Issue.
#pragma once
#include <base/xstring.h>
#include <entitlement/license.h>

namespace entitlement {

struct LicenseInfo {
  u64 expiry_date;
  u64 start_date;
  base::String licensee_name;
  // Optional payload
  base::String payload;
};

// Returns base64 encoded data so it also remains human readable in the .key
// file
base::String IssueLicense(const LicenseInfo& license_info,
    const base::StringRef master_key,
                          base::String& secret);
/*   License issue(const std::string& licensee,
                  unsigned int validityPeriod,
                  const std::string& masterKey,
                  const std::string& secret = "",
                  const std::string& licenseeSignature = "",
                  const std::string& additionalPayload = "") const;*/

}  // namespace entitlement