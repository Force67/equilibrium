// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/span.h>
#include <base/check.h>
#include <base/strings/string_ref.h>

namespace crypto {

bool Base64Encode(const base::Span<byte> in_data, base::String& out);
bool Base64Decode(const base::StringRef in_text, base::String& out);
}  // namespace crypto