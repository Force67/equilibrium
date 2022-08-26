// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Safe wrappers around lowlevel mbedTLS libraries.
#pragma once

#include <base/arch.h>
#include <mbedtls/entropy.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/ctr_drbg.h>

namespace crypto {
#define MBED_VERIFY_RESULT(result, name, return_what) \
  if (result != 0) {                                  \
    LOG_ERROR(#name " failed");                       \
    return return_what;                               \
  }  // namespace entitlement

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

}  // namespace crypto