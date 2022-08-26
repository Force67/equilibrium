// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/strings/string_ref.h>
#include <entitlement/keystone/license_info_spec.h>

namespace base {
class Path;
}

namespace entitlement {
class KeyStoneDatabase;
}

namespace entitlement::issuing_tool {
bool WriteAndFormatLicenseFile(const base::StringRef program_name,
                               const base::StringRef issuing_authority,
                               const base::Path&,
                               KeyStoneDatabase&);
}  // namespace entitlement::issuing_tool