// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/spinning_mutex.h>

#if (OS_WIN)
#include <intrin.h>
#define PA_YIELD_PROCESSOR _mm_pause()
#endif

namespace {
template <class T>
const T& XX_min(const T& a, const T& b) {
  return (b < a) ? b : a;
}
}  // namespace

namespace base {

void SpinningMutex::Reinit() {
#if !defined(OS_MACOSX)
  // On most platforms, no need to re-init the lock, can just unlock it.
  Release();
#else
  unfair_lock_ = OS_UNFAIR_LOCK_INIT;
#endif  // BUILDFLAG(IS_APPLE)
}

void SpinningMutex::AcquireSpinThenBlock() {
  int tries = 0;
  int backoff = 1;
  do {
    if (Try()) [[likely]]
      return;
    // Note: Per the intel optimization manual
    // (https://software.intel.com/content/dam/develop/public/us/en/documents/64-ia-32-architectures-optimization-manual.pdf),
    // the "pause" instruction is more costly on Skylake Client than on previous
    // architectures. The latency is found to be 141 cycles
    // there (from ~10 on previous ones, nice 14x).
    //
    // According to Agner Fog's instruction tables, the latency is still >100
    // cycles on Ice Lake, and from other sources, seems to be high as well on
    // Adler Lake. Separately, it is (from
    // https://agner.org/optimize/instruction_tables.pdf) also high on AMD Zen 3
    // (~65). So just assume that it's this way for most x86_64 architectures.
    //
    // Also, loop several times here, following the guidelines in section 2.3.4
    // of the manual, "Pause latency in Skylake Client Microarchitecture".
    for (int yields = 0; yields < backoff; yields++) {
      PA_YIELD_PROCESSOR;
      tries++;
    }
    constexpr int kMaxBackoff = 16;
    backoff = XX_min(kMaxBackoff, backoff << 1);
  } while (tries < kSpinCount);

  LockSlow();
}

}  // namespace base