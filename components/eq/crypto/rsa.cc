// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "rsa.h"
#include "message_digest.h"
#include "mbed_raii_wrappers.h"	

// see
// https://github.com/wulmx/qemu-6.2/blob/a67f22c921e0e1aa024f4e78dbfc0e421cf7f718/roms/skiboot/libstb/crypto/mbedtls/programs/pkey/pk_sign.c
// for more mbed documentation

namespace crypto {
static constexpr DigestType kMdTypeAlgo{DigestType::MBEDTLS_MD_SHA512};

bool RSASign(PublicKeyWrap& public_key,
             EntropyWrap& entropy,
             CrtDrbgWrap& random,
             const base::Span<byte> input,
             base::Vector<byte>& output) {

  base::Vector<byte> hash;
  bool b = CalculateDigest(kMdTypeAlgo, input, hash);
  MBED_VERIFY_RESULT2(b, CalculateDigest, false);

  mem_size out_size = 0;
  output.resize(MBEDTLS_MPI_MAX_SIZE);

  b = public_key.Sign(kMdTypeAlgo, hash, output, random);
  MBED_VERIFY_RESULT2(b, public_key.Sign, false);

  return output.size() > 0;
}

bool RSAVerify(PublicKeyWrap& public_key,
               const base::Span<byte> input,
               const base::Span<byte> signature) {
  base::Vector<byte> hash;
  bool b = CalculateDigest(kMdTypeAlgo, input, hash);
  MBED_VERIFY_RESULT2(b, CalculateDigest, false);

  bool result = public_key.Verify(kMdTypeAlgo, hash, signature);
  MBED_VERIFY_RESULT2(result, public_key.Verify, false);
  return true;
}
}  // namespace crypto