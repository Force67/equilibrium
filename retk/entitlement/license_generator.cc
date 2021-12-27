// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Licensing authority: Issue.

#include <mbedtls/entropy.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>

#include <base/logging.h>
#include <base/random.h>

#include "entitlement/license_generator.h"

namespace entitlement {

// safe_crypto_wrappers.cc

namespace {
class EntropyWrap {
 public:
  EntropyWrap() { mbedtls_entropy_init(&context_); }
  ~EntropyWrap() { mbedtls_entropy_free(&context_); }

  auto* get() { return &context_; }

 private:
  mbedtls_entropy_context context_;
};

class CrtDrbgWrap {
 public:
  CrtDrbgWrap() { mbedtls_ctr_drbg_init(&context_); }
  ~CrtDrbgWrap() { mbedtls_ctr_drbg_free(&context_); }

  auto* get() { return &context_; }

  void Seed(EntropyWrap& entropy, const u8* data, size_t len) {
    mbedtls_ctr_drbg_seed(&context_, mbedtls_entropy_func,
                          reinterpret_cast<void*>(entropy.get()), data, len);
  }

 private:
  mbedtls_ctr_drbg_context context_;
};

class PkWrap {
 public:
  PkWrap() { mbedtls_pk_init(&context_); }
  ~PkWrap() { mbedtls_pk_free(&context_); }

  auto* get() { return &context_; }

 private:
  mbedtls_pk_context context_;
};

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

template <typename T>
class FixedPointer {
 public:
  FixedPointer(const T* ptr, size_t length) : ptr_(ptr), length_(length) {}

  const T* get() const { return ptr_; }
  size_t length() const { return length_; }

 private:
  T* ptr_;
  size_t length_;
};

char EncodeProductCode() {
  return ' ';
}
// So anyway, what features should a license have?
// Probably a version, so we start with that
// i8 version
// 

struct License {
  // Base64 begin:
  i8 version;
  i8 type;
  u16 reserved;
  u64 start_date;
  u64 end_date;
  // These are all rounded.
  base::String issuing_authority; // VH-Tech by default
  base::String licensee_name;
  base::String payload;
  // Base64 end

  // Base64 begin:
  // Signature to prevent alteration
  u8 signature[32];
  // Base64 end
};

void SignData(const FixedPointer<u8> data,
              // Private key buffer
              const FixedPointer<u8> private_key,
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