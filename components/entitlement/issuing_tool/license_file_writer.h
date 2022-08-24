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
// exposed for testing.
base::String EncodeLicenseBlock(const LicenseHeader& header,
                                const base::StringU8& licensee_name);

void WriteAndFormatLicenseFile(const base::Path&, KeyStoneDatabase&);
}  // namespace entitlement::issuing_tool