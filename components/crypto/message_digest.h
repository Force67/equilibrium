// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/vector.h>
#include <crypto/mbed_raii_wrappers.h>

#include <external/mbedtls/library/md_wrap.h> // for md_info struct

namespace crypto {

using DigestType = mbedtls_md_type_t;

inline const mbedtls_md_info_t* MessageDigestInfoFromType(DigestType dt) {
  return mbedtls_md_info_from_type(dt);
}

inline bool CalculateDigest(DigestType digest_type,
                                 const base::Span<byte> input,
                                 base::Vector<byte>& out_hash) {
  const mbedtls_md_info_t* md_info = MessageDigestInfoFromType(digest_type);
  out_hash.resize(md_info->size);  // very important else mbed craps itself

  int r = mbedtls_md(md_info, input.data(), input.size(), out_hash.data());
  MBED_VERIFY_RESULT(r, mbedtls_md, false);
  return true;
}
}  // namespace crypto