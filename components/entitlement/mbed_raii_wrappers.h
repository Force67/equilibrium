// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Safe wrappers around lowlevel mbedTLS libraries.
#pragma once

#include <base/arch.h>
#include <mbedtls/entropy.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>

namespace entitlement {
class EntropyWrap {
 public:
  EntropyWrap() { mbedtls_entropy_init(&context_); }
  ~EntropyWrap() { mbedtls_entropy_free(&context_); }

  auto& context() { return context_; }

 private:
  mbedtls_entropy_context context_;
};

class CrtDrbgWrap {
 public:
  CrtDrbgWrap() { mbedtls_ctr_drbg_init(&context_); }
  ~CrtDrbgWrap() { mbedtls_ctr_drbg_free(&context_); }

  auto& context() const { return context_; }

  void Seed(EntropyWrap& entropy, const u8* data, size_t len) {
    mbedtls_ctr_drbg_seed(&context_, mbedtls_entropy_func,
                          reinterpret_cast<void*>(&entropy.context()), data, len);
  }

 private:
  mbedtls_ctr_drbg_context context_;
};

class PublicKeyWrap {
 public:
  PublicKeyWrap() { mbedtls_pk_init(&context_); }
  ~PublicKeyWrap() { mbedtls_pk_free(&context_); }

  auto& context() { return context_; }

 private:
  mbedtls_pk_context context_;
};

int Base64Encode(base::String& out, const base::Span<u8> in_data) {
  return mbedtls_base64_encode(reinterpret_cast<u8*>(out.data()), out.length(),
                               nullptr, reinterpret_cast<const u8*>(in_data.data()),
                               in_data.length());
}

}  // namespace entitlement