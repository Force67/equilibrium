// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/path.h>

#include <base/filesystem/file.h>

#include <fmt/format.h>
#include <base/math/alignment.h>
#include <crypto/base64.h>
#include <crypto/mbed_raii_wrappers.h>
#include <base/optional.h>
#include <base/time/time.h>
#include <entitlement/issuing_tool/license_file_writer.h>
#include <entitlement/keystone/keystone_database.h>

#include <flatbuffers/flatbuffers.h>
#include <entitlement/protocol/license_generated.h>

namespace entitlement::issuing_tool {

// or we serialize a flatbuffer?
// license_block_writer.cc
base::Optional<base::String> EncodeLicenseBlock(
    const LicenseHeader& header,
    const base::StringRef program_name,
    const base::StringRef issuing_authority,
    const base::StringU8& licensee_name,
    KeyStoneDatabase& keystone_database) {

  flatbuffers::FlatBufferBuilder fbb;
  auto license_id = fbb.CreateString("AAA-BBB-CCC-DDD");
  
  base::Vector<flatbuffers::Offset<entitlement::Feature>> feature_vector(
      keystone_database.entitlements().size(),
      base::VectorReservePolicy::kForPushback);
  for (const FeatureEntilement& fe : keystone_database.entitlements()) {
    const auto feature =
        entitlement::CreateFeature(fbb, 1, 1, fbb.CreateString(fe.license_id),
                                   fe.feature_id, fe.support_expiry_timestamp);
    feature_vector.push_back(feature);
  }
  auto feature_set = fbb.CreateVector(feature_vector.data(), feature_vector.size());

  auto lb = entitlement::CreateLicenseBlock(
      fbb, 1, 1, entitlement::LicenseType_Regular, license_id,
      header.issue_timestamp, header.expiry_timestamp, feature_set);
  fbb.Finish(lb);

  base::String base64_contents;
  if (!crypto::Base64Encode(base::Span(fbb.GetBufferPointer(), fbb.GetSize()),
                            base64_contents))
    return {};

  // scramble the base64
  // for (mem_size i = 0; i < base64_contents.size(); i++) {
  //  base64_contents[i] ^= i % 256;
  // }

  return base64_contents;

  // begin base64
  // u8 signature[32];  //< Signature to prevent alteration
  // end base64
}

bool WriteAndFormatLicenseFile(const base::StringRef program_name,
                               const base::StringRef issuing_authority,
                               const base::Path& location,
                               KeyStoneDatabase& keystone_database) {
  base::String s = fmt::format("{} {}\n", program_name.c_str(),
                               keystone_database.header().program_version);
  for (const FeatureEntilement& fe : keystone_database.entitlements()) {
    s += fmt::format("{}:{}\n", fe.license_id, fe.support_expiry_timestamp);
  }
  s += "\n";
  // data block
  auto encode_result =
      EncodeLicenseBlock(keystone_database.header(), program_name, issuing_authority,
                         keystone_database.licensee(), keystone_database);
  if (encode_result.failed())
    return false;

  s += encode_result.value();
  {
    base::File f(location, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    f.Write(0, s.c_str(), s.length());
  }
  return true;
}
}  // namespace entitlement::issuing_tool