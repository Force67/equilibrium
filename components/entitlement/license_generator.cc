// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Licensing authority: Issue.

#include <base/logging.h>
#include <base/random.h>

#include <base/containers/span.h>

#include "entitlement/license_generator.h"

namespace entitlement {

// safe_crypto_wrappers.cc

namespace {



static base::String MBedError() {  // mbedtls_strerror()
  return {};
}

}  // namespace

// base: is_module_confidiental
// Disables the error output in release entirely

#define MBED_CHECK_ERR(name)    \
  if (result != 0) {            \
    LOG_ERROR(#name " failed"); \
    return;                     \
  }  // namespace entitlement

// So anyway, what features should a license have?
// Probably a version, so we start with that
// i8 version
// 

// entitlement_block.cc
// key_file_writer.cc


void SignData(const base::Span<u8> data,
              // Private key buffer
              const base::Span<u8> private_key,
              // PEM
              const base::String& priv_key_pem) {
  EntropyWrap entropy{};
  PkWrap pk_context{};

  CrtDrbgWrap random{};

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

  // Calculate the SHA hash of given data.
  // Is the digest the private key?
  u8 digest[32]{};
  result = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), data.get(),
                      data.length(), digest);
  MBED_CHECK_ERR(mbedtls_md);

  size_t out_len = 0;
  u8 out_buf[512]{};
  result = mbedtls_pk_sign(pk_context.get(), MBEDTLS_MD_SHA256, &digest[0],
                           sizeof(digest), &out_buf[0], sizeof(out_buf),
                           &out_len, mbedtls_ctr_drbg_random, random.get());
  MBED_CHECK_ERR(mbedtls_pk_sign);
}
// Maybe we do need a license factory with methods for creating Text, binary, flat buffer license.. 

// The authority id is VHT for personal licenses or {CompanyName} (that bought
// the product) for floating Licenses
base::String IssueLicense(const LicenseInfo& license_info,
                          const base::StringRef master_key,
                          base::String& secret) {
  

  base::String license_text =
      fmt::format("{}:{},{},{},{},{}", license_info.start_date,
                  license_info.expiry_date, license_info.licensee_name, license_info.payload);
  // Compute the signature of the data above and store it behind
  SignData({reinterpret_cast<const u8*>(license_text.c_str()),
            license_text.length()},
           {nullptr, 0}, secret);

  base::String base_string;
  base_string.resize(4096);
  int result = mbedtls_base64_encode(
      reinterpret_cast<u8*>(base_string.data()), base_string.length(), nullptr,
      reinterpret_cast<const u8*>(license_text.c_str()), license_text.length());

  return base_string;
}
}  // namespace entitlement