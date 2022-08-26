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

namespace entitlement::issuing_tool {

namespace {
class BufferedWriter {
 public:
  BufferedWriter(mem_size size_est)
      : data_(size_est / sizeof(byte), base::VectorReservePolicy::kForData){};
  ~BufferedWriter() = default;

  template <typename T>
  void PutS(const base::BasicStringRef<T>& string) {
    if (offset_ + string.length() > data_.size())
      __debugbreak();

    *reinterpret_cast<u16*>(&data_[offset_]) = string.length();
    offset_ += sizeof(u16);
    memcpy(&data_[offset_], string.data(), string.length());
    offset_ += string.length();
  }

  template <typename T>
  void Put(const T& value) {
    if ((offset_ + sizeof(T)) > data_.size())
      __debugbreak();

    *reinterpret_cast<T*>(&data_[offset_]) = value;
    offset_ += sizeof(T);
  }

  auto span() {
    return base::Span{reinterpret_cast<const byte*>(data_.data()), data_.size()};
  }

 private:
  mem_size offset_{0};
  base::Vector<byte> data_;
};

// this routine belongs in crypto module?
void SignData(const base::Span<u8> data,
              // Private key buffer
              const base::Span<u8> private_key,
              // PEM
              const base::String& priv_key_pem) {
  crypto::EntropyWrap entropy{};
  crypto::PublicKeyWrap pk_context{};

  crypto::CrtDrbgWrap random{};

  // TODO: This is not proper entropy at all
  u32 random_number = base::RandomUint();
  random.Seed(entropy, reinterpret_cast<const u8*>(&random_number),
              sizeof(random_number));

  // Parse the private key
  int result = mbedtls_pk_parse_key(
      pk_context.get(), private_key.get(), private_key.length(),
      reinterpret_cast<const u8*>(priv_key_pem.c_str()), priv_key_pem.length(),
      mbedtls_ctr_drbg_random, random.get());
  MBED_CHECK_ERR(mbedtls_pk_parse_key);

  // https://github.com/iLuSIAnn/testing/blob/b8b735c9e4141eb39562d142e61b06f89b23a475/src/tls/hash.h

  // Calculate the SHA hash of given data.
  // Is the digest the private key?
  u8 digest[32]{};
  result = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), data.get(),
                      data.length(), digest);
  MBED_CHECK_ERR(mbedtls_md);

  size_t out_len = 0;
  u8 out_buf[512]{};
  result = mbedtls_pk_sign(pk_context.get(), MBEDTLS_MD_SHA256, &digest[0],
                           sizeof(digest), &out_buf[0], sizeof(out_buf), &out_len,
                           mbedtls_ctr_drbg_random, random.get());
  MBED_CHECK_ERR(mbedtls_pk_sign);
}
}  // namespace

// or we serialize a flatbuffer?
// license_block_writer.cc
base::Optional<base::String> EncodeLicenseBlock(
    const LicenseHeader& header,
    const base::StringRef program_name,
    const base::StringRef issuing_authority,
    const base::StringU8& licensee_name) {
  const mem_size kSizeEst = base::Align<mem_size>((sizeof(LicenseHeader) +
                            (program_name.length() * sizeof(char)) +
                            (issuing_authority.length() * sizeof(char)) +
                            (licensee_name.length() * sizeof(char8_t))), 16);

  BufferedWriter writer(kSizeEst);
  writer.Put(header);
  // TODO: string table.
  writer.PutS(program_name);
  writer.PutS(issuing_authority);
  writer.PutS<char8_t>(licensee_name);

  base::String base64_contents;
  if (!crypto::Base64Encode(writer.span(), base64_contents))
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
                         keystone_database.licensee());
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