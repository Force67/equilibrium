// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <crypto/rsa.h>
#include <crypto/mbed_raii_wrappers.h>

#include <external/mbedtls/library/md_wrap.h>
#include <base/random.h>
#include <mbedtls/pk.h>

// see
// https://github.com/wulmx/qemu-6.2/blob/a67f22c921e0e1aa024f4e78dbfc0e421cf7f718/roms/skiboot/libstb/crypto/mbedtls/programs/pkey/pk_sign.c
// for more mbed documentation

namespace crypto {
static constexpr mbedtls_md_type_t kMdTypeAlgo{MBEDTLS_MD_SHA512};

bool RSASign(PublicKeyWrap& public_key,
             EntropyWrap& entropy,
             CrtDrbgWrap& random,
             const base::Span<byte> input,
             base::Vector<byte>& output) {
  const mbedtls_md_info_t* algo_info = mbedtls_md_info_from_type(kMdTypeAlgo);
  MBED_VERIFY_RESULT2(algo_info, mbedtls_md_info_from_type, false);

  base::Vector<byte> hash(algo_info->size, base::VectorReservePolicy::kForData);

  int r = mbedtls_md(algo_info, input.data(), input.size(), hash.data());
  MBED_VERIFY_RESULT(r, mbedtls_md, false);

  mem_size out_size = 0;
  output.resize(MBEDTLS_MPI_MAX_SIZE);

  bool b = public_key.Sign(kMdTypeAlgo, hash, output, random);
  MBED_VERIFY_RESULT2(b, public_key.Sign, false);

  return output.size() > 0;
}

bool RSAVerify(PublicKeyWrap& public_key,
               const base::Span<byte> input,
               const base::Span<byte> signature) {
  const mbedtls_md_info_t* info = mbedtls_md_info_from_type(kMdTypeAlgo);
  base::Vector<byte> hash(info->size, base::VectorReservePolicy::kForData);

  int r = mbedtls_md(info, input.data(), input.size(), hash.data());
  MBED_VERIFY_RESULT(r, mbedtls_md, false);

  bool result = public_key.Verify(kMdTypeAlgo, hash, signature);
  MBED_VERIFY_RESULT2(result, public_key.Verify, false);
  return true;
}
}  // namespace crypto