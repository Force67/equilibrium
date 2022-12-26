// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Safe wrappers around lowlevel mbedTLS libraries.
#pragma once

#include <base/arch.h>
#include <base/logging.h>
#include <base/containers/span.h>

#include <mbedtls/entropy.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/ctr_drbg.h>

namespace crypto {
inline const char* TranslateMbedError(int e) {
  switch (e) {
    case MBEDTLS_ERR_PK_ALLOC_FAILED:
      return "Memory allocation failed.";
    case MBEDTLS_ERR_PK_TYPE_MISMATCH:
      return "Type mismatch, eg attempt to encrypt with an ECDSA key";
    case MBEDTLS_ERR_PK_BAD_INPUT_DATA:
      return "Bad input parameters to function.";
    case MBEDTLS_ERR_PK_FILE_IO_ERROR:
      return "Read/write of file failed.";
    case MBEDTLS_ERR_PK_KEY_INVALID_VERSION:
      return "Unsupported key version.";
    case MBEDTLS_ERR_PK_KEY_INVALID_FORMAT:
      return "Invalid key tag or value.";
    case MBEDTLS_ERR_PK_UNKNOWN_PK_ALG:
      return "Key algorithm is unsupported (only RSA and EC are supported).";
    case MBEDTLS_ERR_PK_PASSWORD_REQUIRED:
      return "Private key password can't be empty.";
    case MBEDTLS_ERR_PK_PASSWORD_MISMATCH:
      return "Given private key password does not allow for correct decryption.";
    case MBEDTLS_ERR_PK_INVALID_PUBKEY:
      return "The pubkey tag or value is invalid (only RSA and EC are supported).";
    case MBEDTLS_ERR_PK_INVALID_ALG:
      return "The algorithm tag or value is invalid.";
    case MBEDTLS_ERR_PK_UNKNOWN_NAMED_CURVE:
      return "Elliptic curve is unsupported (only NIST curves are supported).";
    case MBEDTLS_ERR_PK_FEATURE_UNAVAILABLE:
      return "Unavailable feature, e.g. RSA disabled for RSA key.";
    case MBEDTLS_ERR_PK_SIG_LEN_MISMATCH:
      return "The buffer contains a valid signature followed by more data. ";
    case MBEDTLS_ERR_PK_BUFFER_TOO_SMALL:
      return "The output buffer is too small.";
    case MBEDTLS_ERR_RSA_BAD_INPUT_DATA:
      return "Bad input parameters to function.";
    case MBEDTLS_ERR_RSA_INVALID_PADDING:
      return "Input data contains invalid padding and is rejected.";
    case MBEDTLS_ERR_RSA_KEY_GEN_FAILED:
      return "Something failed during generation of a key.";
    case MBEDTLS_ERR_RSA_KEY_CHECK_FAILED:
      return "Key failed to pass the validity check of the library.";
    case MBEDTLS_ERR_RSA_PUBLIC_FAILED:
      return "The public key operation failed.";
    case MBEDTLS_ERR_RSA_PRIVATE_FAILED:
      return "The private key operation failed.";
    case MBEDTLS_ERR_RSA_VERIFY_FAILED:
      return "The PKCS#1 verification failed.";
    case MBEDTLS_ERR_RSA_OUTPUT_TOO_LARGE:
      return "The output buffer for decryption is not large enough.";
    case MBEDTLS_ERR_RSA_RNG_FAILED:
      return "The random generator failed to generate non-zeros.";
    default:
      return "<unknown>";
  }
}

#define MBED_VERIFY_RESULT(result, name, return_what)    \
  if (result != 0) {                                     \
    LOG_ERROR(#name ": {}", TranslateMbedError(result)); \
    return return_what;                                  \
  }

#define MBED_VERIFY_RESULT2(result, name, return_what) \
  if (!result) {                                       \
    LOG_ERROR(#name ": {}", #result);                  \
    return return_what;                                \
  }

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

  auto& context() { return context_; }

  bool Seed(EntropyWrap& entropy, const base::Span<byte> entropy_data) {
    int r = mbedtls_ctr_drbg_seed(&context_, mbedtls_entropy_func,
                                  reinterpret_cast<void*>(&entropy.context()),
                                  entropy_data.data(), entropy_data.length());
    MBED_VERIFY_RESULT(r, mbedtls_ctr_drbg_seed, false);
    return true;
  }

 private:
  mbedtls_ctr_drbg_context context_;
};

class PublicKeyWrap {
 public:
  PublicKeyWrap() { mbedtls_pk_init(&context_); }
  ~PublicKeyWrap() { mbedtls_pk_free(&context_); }

  auto& context() { return context_; }

  bool ParseKeyFile(const char* path, CrtDrbgWrap& wrap) {
    int r = mbedtls_pk_parse_keyfile(
        &context_, path, nullptr, mbedtls_ctr_drbg_random, (void*)&wrap.context());
    MBED_VERIFY_RESULT(r, mbedtls_pk_parse_keyfile, false);
    return false;
  }

  bool ParsePublicKey(const base::Span<byte> key) {
    int r = mbedtls_pk_parse_public_key(&context_, key.data(), key.size());
    MBED_VERIFY_RESULT(r, mbedtls_pk_parse_public_key, false);
    return true;
  }

  bool ParsePrivateKey(const base::Span<byte> key, CrtDrbgWrap& wrap) {
    int r = mbedtls_pk_parse_key(&context_, key.data(), key.length(), nullptr, 0,
                                 mbedtls_ctr_drbg_random, (void*)&wrap.context());
    MBED_VERIFY_RESULT(r, mbedtls_pk_parse_key, false);
    return true;
  }

  bool Sign(const mbedtls_md_type_t type,
            const base::Span<byte> hash,
            base::Vector<byte>& buf,
            CrtDrbgWrap& dbg_wrap) {
    void* random_context = reinterpret_cast<void*>(&dbg_wrap.context());

    size_t olen = 0;
    int r = mbedtls_pk_sign(
        &context_, type /* Hash algorithm used*/,
        hash.data() /*Hash of the message to sign*/, hash.size() /*Hash length*/,
        buf.data() /*Place to write the signature*/,
        buf.size() /*The size of the \p sig buffer in bytes*/,
        &olen, /*On successful return,
                *                  the number of bytes written to \p sig.*/
        mbedtls_ctr_drbg_random, random_context);
    MBED_VERIFY_RESULT(r, mbedtls_pk_sign, false);

    buf.resize(olen);
    return true;
  }

  bool Verify(const mbedtls_md_type_t type,
              const base::Span<byte> hash,
              const base::Span<byte> signature) {
    int r = mbedtls_pk_verify(&context_, type, hash.data(), hash.size(),
                              signature.data(), signature.length());
    MBED_VERIFY_RESULT(r, mbedtls_pk_verify, false);
    return true;
  }

 private:
  mbedtls_pk_context context_;
};

}  // namespace crypto