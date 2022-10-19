// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <mbedtls/base64.h>
#include "base64.h"
#include <cstddef>

namespace crypto {
bool Base64Encode(const base::Span<byte> in_data, base::String& out) {
  DCHECK(!in_data.empty(), "Invalid span passed to Base64Encode");

  size_t out_length = 0;
  int result = 0;

  // reasonable default, so that we dont have to resize for each character in the
  // beginning
  out.resize(256);

  do {
    result = mbedtls_base64_encode(
        reinterpret_cast<byte*>(out.data()), out.length(), &out_length,
        reinterpret_cast<const byte*>(in_data.data()), in_data.length());
    if (result == 0)
      break;

    // WARNING: resize takes character length, not byte length
    out.resize(out.length() * 2);
  } while (result == MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL &&
           out_length <= (mem_size)-1);

  // this fixes issues where we have an excess of 0 bytes at the end being counted as
  // part of the string
  out.resize(out_length);

  return result == 0;
}

bool Base64Decode(const base::StringRef in_text, base::String& out) {
  size_t out_length = 0;
  int result = 0;

  // reasonable default, so that we dont have to resize for each character in the
  // beginning
  out.resize(256);

  do {
    result = mbedtls_base64_decode(
        reinterpret_cast<byte*>(out.data()), out.length(), &out_length,
        reinterpret_cast<const byte*>(in_text.data()), in_text.length());
    if (result == 0)
      break;

    out.resize(out.length() * 2);

  } while (result == MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL &&
           out_length <= (mem_size)-1);

  out.resize(out_length);

  return result == 0;
}
}  // namespace crypto