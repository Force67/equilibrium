// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "random.h"

#include <Windows.h>
#include <Wincrypt.h>

namespace base {

u64 SourceTrueRandomSeed() {
  HCRYPTPROV hProvider = 0;
  if (!::CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    return 0;
  }
  u64 seed = 0;
  BOOL result =
      ::CryptGenRandom(hProvider, sizeof(seed), reinterpret_cast<BYTE*>(&seed));
  ::CryptReleaseContext(hProvider, 0);
  if (!result)
    return 0;
  return seed;
}
}  // namespace base