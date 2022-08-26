// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Safe wrappers around lowlevel mbedTLS libraries.
#pragma once

#include <mbedtls/pk.h>
#include <crypto/mbed_raii_wrappers.h>

#include <base/containers/span.h>
#include <base/containers/vector.h>

#include <base/strings/string_ref.h>

namespace crypto {

class PrivateKey {
 public:
  PrivateKey();
  ~PrivateKey();

  bool Parse(const base::StringRef key, const base::StringRef key_pem);
  bool Sign(const base::Span<byte> digest, base::Vector<byte>& out);

 private:
  mbedtls_ctr_drbg_context random_;
  mbedtls_pk_context context_;
};

PrivateKey::PrivateKey() {
  mbedtls_ctr_drbg_init(&random_);
  mbedtls_pk_init(&context_);
}

PrivateKey::~PrivateKey() {
  mbedtls_pk_free(&context_);
  mbedtls_ctr_drbg_free(&random_);
}

bool PrivateKey::Parse(const base::StringRef key, const base::StringRef key_pem) {
  // key_txt.length() is increased by 1 as it does not include the NULL-terminator
  // which mbedtls_pk_parse_key() expects to see.
  const int r = mbedtls_pk_parse_key(
      &context_, reinterpret_cast<const byte*>(key.c_str()), key.length() + 1,
      reinterpret_cast<const byte*>(key_pem.c_str()), key_pem.length() + 1,
      mbedtls_ctr_drbg_random, &random_);
  return r >= 0;
}

// https://github.com/Kirill380/kaa/blob/master/client/client-multi/client-c/src/kaa/utilities/kaa_aes_rsa.c#L94

// mbedtls_pk_sign
// https://cpp.hotexamples.com/examples/-/-/mbedtls_pk_sign/cpp-mbedtls_pk_sign-function-examples.html#0x44175b7d1fb61013e065a903c5e74c664e65622bcc86ecbee180257881053ade-425,,461,
bool PrivateKey::Sign(const base::Span<byte> digest,
                      base::Vector<byte>& out_signature) {
  mem_size out_len = 0;
  const int r = mbedtls_pk_sign(&context_, MBEDTLS_MD_SHA256, digest.data(), digest.size(),
                  out_signature.data(), out_signature.size(), &out_len,
                  mbedtls_ctr_drbg_random, &random_);
  return r >= 0;
}

}  // namespace crypto