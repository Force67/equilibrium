// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <entitlement/license.h>

namespace base {
class Path;
}

namespace entitlement {
// exposed for testing.
base::String EncodeLicenseBlock(const LicenseHeader& header,
                                const base::StringU8& licensee_name,
                                const base::StringRefU8 optional_additional_data);

void WriteLicenseFile(const base::Path&);
}  // namespace entitlement