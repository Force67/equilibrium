// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/path.h>

#include <base/filesystem/file.h>

#include <fmt/format.h>

#include <crypto/base64.h>
#include <base/time/time.h>
#include <entitlement/issuing_tool/license_file_writer.h>
#include <entitlement/keystone/keystone_database.h>

namespace entitlement::issuing_tool {
namespace {
constexpr const char8_t kIssuingAuthority[]{u8"VH-TECH"};
constexpr const char8_t kLicenseHead[]{u8"License XXX.XXX.XXX LMAO?"};

}  // namespace

// consider the license name as an extended "field"
base::String EncodeLicenseBlock(const LicenseHeader& header,
                                const base::StringU8& licensee_name) {
  base::String header_text;
  const base::Span header_span(reinterpret_cast<const byte*>(&header),
                               sizeof(LicenseHeader));
  if (!crypto::Base64Encode(header_span, header_text))
    return {};

  base::String name_text;
  const base::Span name_span{reinterpret_cast<const byte*>(licensee_name.data()),
                             licensee_name.length()};
  if (!crypto::Base64Encode(name_span, name_text))
    return {};

  return header_text + name_text;

  // begin base64
  // u8 signature[32];  //< Signature to prevent alteration
  // end base64
}

void WriteAndFormatLicenseFile(const base::Path& location,
                               KeyStoneDatabase& keystone_database) {
  base::File f(location, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  // copyright
  mem_size pos = f.Write(0, reinterpret_cast<const char*>(kLicenseHead), sizeof(kLicenseHead));

  // produce fancy text
  for (const FeatureEntilement& fe : keystone_database.entitlements()) {
    auto text = fmt::format("{}:{}\n", fe.license_id, fe.support_expiry_timestamp);
    pos += f.Write(pos, text.c_str(), text.length());
  }

  // data block
  base::String license_block =
      EncodeLicenseBlock(keystone_database.header(), keystone_database.licensee());
  pos += f.Write(pos, license_block.c_str(), license_block.length());
}
}  // namespace entitlement::issuing_tool