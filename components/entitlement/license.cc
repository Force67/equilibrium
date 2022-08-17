// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Licensing data.

#include <entitlement/license.h>
#include <entitlement/mbed_raii_wrappers.h>
#include <base/containers/vector.h>
#include <base/math/math_helpers.h>

#include <base/math/alignment.h>

constexpr auto x = base::Align(65, 4);

namespace entitlement {
namespace {
constexpr const char8_t kIssuingAuthority[]{u8"VH-TECH"};
}  // namespace

base::String EncodeLicenseBlock(
    const LicenseHeader& header,
    const base::StringU8& licensee_name,
    const base::StringRefU8 optional_additional_data) {
  base::String storage;
  // first comes the license header
  *reinterpret_cast<LicenseHeader*>(buffer.data()) = header;

  // https://github.com/amrayn/licensepp/blob/master/src/issuing-authority.cc#L113
  auto offset = WritePaddedString<char8_t>(buffer.data() + sizeof(LicenseHeader),
                                           kIssuingAuthority);
  offset += WritePaddedString<char8_t>(buffer.data() + offset, licensee_name);
  offset +=
      WritePaddedString<char8_t>(buffer.data() + offset, optional_additional_data);

  base::String base64_result;
  int result = Base64Encode(base64_result, base::Span(buffer.data(), buffer.size()));

  // begin base64
  u8 signature[32];  //< Signature to prevent alteration
  // end base64
}
}  // namespace entitlement