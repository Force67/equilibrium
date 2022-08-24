// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/path.h>

#include <entitlement/license.h>
#include <entitlement/license_file_writer.h>
#include <entitlement/mbed_raii_wrappers.h>

namespace entitlement {
namespace {
constexpr const char8_t kIssuingAuthority[]{u8"VH-TECH"};
}  // namespace

base::String EncodeLicenseBlock(const LicenseHeader& header,
                                const base::StringU8& licensee_name,
                                const base::StringRefU8 optional_additional_data) {
  base::String storage;
  storage.resize(2048);
  {
    const base::Span header_span(reinterpret_cast<const byte*>(&header),
                                 sizeof(LicenseHeader));
    size_t out_length = 0;
    MBED_VERIFY_RESULT(entitlement::Base64Encode(out_length, storage, header_span),
                       "Base64Encode", {});
  }

  {

  }


  // begin base64
  //u8 signature[32];  //< Signature to prevent alteration
  // end base64

  return {};
}

void WriteLicenseFile(const base::Path& location) {}
}  // namespace entitlement