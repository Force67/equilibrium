// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/file.h>

// crypto
#include <base/random.h>
#include <crypto/base64.h>
#include <crypto/rsa.h>
#include <crypto/mbed_raii_wrappers.h>

// ent library
#include <entitlement/keystone/keystone_database.h>
#include <entitlement/issuing_tool/key_vault.h>

// auto generated
#include <protocol/license_generated.h>

static constexpr u16 kFeatureStructVersion = 1;
static constexpr u16 kLicenseStructVersion = 1;

namespace issuing_tool {
bool WriteAndFormatLicenseFile(const base::Path& location,
                               const base::String& license_data,
                               const base::StringRef program_name,
                               uint32_t program_version) {
  base::String s = fmt::format("{} {}\n", program_name.c_str(), program_version);
  s += license_data;
  {
    base::File f(location, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    if (!f.IsValid())
      return false;
    f.Write(0, s.c_str(), s.length());
  }
  return true;
}

bool SignData(const base::Span<byte> data, base::Vector<byte>& signage) {
  crypto::PublicKeyWrap pk;
  crypto::EntropyWrap entropy;
  crypto::CrtDrbgWrap random;

  u64 entropy_data[8]{};
  for (auto i = 0; i < 8; i++) {
    entropy_data[i] = base::RandomUint() + base::RandomUint();
  }
  if (!random.Seed(entropy, base::Span(reinterpret_cast<byte*>(&entropy_data[0]),
                                       sizeof(u64) * 8)))
    return false;

  const base::Span s(reinterpret_cast<const byte*>(&issuing_tool::kRedKey[0]),
                     sizeof(issuing_tool::kRedKey));

  if (!pk.ParsePrivateKey(s, random))
    return false;

  return crypto::RSASign(pk, entropy, random, data, signage);
}

base::String EncodeAndSignLicenseData(const base::Span<byte> data) {
  // sign available data first.
  base::Vector<byte> signature;
  if (!SignData(data, signature))
    return {};

  base::String base64_content, base64_sign;
  if (!crypto::Base64Encode(data, base64_content) ||
      !crypto::Base64Encode(signature, base64_sign))
    return {};

#if 0
  // scramble the base64 to defeat statistical analysis
  for (mem_size i = 0; i < base64_content.size(); i++) {
    base64_content[i] ^= i % 256;
  }
  for (mem_size i = 0; i < base64_sign.size(); i++) {
    base64_sign[i] ^= i % 256;
  }
#endif

  auto contents = fmt::format("{}:", base64_content.length());
  contents += base64_content;
  contents += fmt::format("\n{}:", base64_sign.length());
  contents += base64_sign;

  return contents;
}
}  // namespace issuing_tool

int main() {
  flatbuffers::FlatBufferBuilder fbb;

  entitlement::LicenseBuilder lb(fbb);
  lb.AddEntitlement("AAA-BBB-CCC", 512, 1000);
  lb.AddEntitlement("BBB-CCC-DDD", 1024, 1000);
  lb.AddEntitlement("EEE-FFF-GGG", 2048, 1000);
  lb.SetIssuingAuthority("VH_TECH");
  lb.SetLicensee("Bethoven");
  lb.SetLicenseID("AAA-BBB_CCC");

  lb.BeginLicense();
  lb.SetLicenseType(entitlement::LicenseType_Regular);
  lb.Finish();

  auto license_data = issuing_tool::EncodeAndSignLicenseData(
      base::Span(fbb.GetBufferPointer(), fbb.GetSize()));
  issuing_tool::WriteAndFormatLicenseFile(R"(Q:\Mods\retk.lk)", license_data, "RETK",
                                          1338);
  return 0;
}