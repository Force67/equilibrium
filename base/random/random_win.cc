// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "random.h"

#include <Windows.h>
#include <Wincrypt.h>

namespace base {

  uint64_t SourceTrueRandomSeed() {
  	HCRYPTPROV hProvider = 0;
  if (!::CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    // Handle error
    return 0;
  }

  uint64_t seed;
  BOOL result = ::CryptGenRandom(hProvider, sizeof(seed), (BYTE*)&seed);

  ::CryptReleaseContext(hProvider, 0);

  if (!result) {
    // Handle error
    return 0;
  }

  return seed;
}
}