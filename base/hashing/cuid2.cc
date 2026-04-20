// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "cuid2.h"
#include "sha256.h"

#include <cstdio>
#include <cstring>

#include <base/atomic.h>
#include <base/random/random.h>
#include <base/time/time.h>

#ifdef __linux__
#include <unistd.h>
#endif

namespace {

static const char kBase36[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static base::Atomic<u64> s_counter{0};

// Encode raw bytes as base36 characters into |out|.
// Writes exactly |out_len| characters (no null terminator).
void EncodeBase36(const u8* bytes, mem_size byte_len, char* out, i32 out_len) {
  // Simple approach: treat each byte as a value and map to base36.
  // For good distribution, combine pairs of bytes and mod 36.
  i32 written = 0;
  mem_size bi = 0;
  while (written < out_len) {
    // Combine two bytes for better spread when available.
    u32 val;
    if (bi + 1 < byte_len) {
      val = (u32(bytes[bi]) << 8) | u32(bytes[bi + 1]);
      bi += 2;
    } else if (bi < byte_len) {
      val = bytes[bi];
      bi += 1;
    } else {
      // Wrap around if we run out of hash bytes (shouldn't happen with
      // SHA-256's 32 bytes for 24 output chars, but be safe).
      bi = 0;
      val = bytes[bi];
      bi += 1;
    }
    out[written++] = kBase36[val % 36];
  }
}

}  // namespace

namespace base {

void GenerateCuid2(char* out) {
  // 1. Gather entropy sources.
  u64 ms = static_cast<u64>(base::GetUnixTimeMilliseconds());
  u64 count = s_counter.fetch_add(1, std::memory_order_relaxed);

  // Kernel CSPRNG is strictly stronger than std::random_device here (the
  // underlying libstdc++ impl is also /dev/urandom on Linux).
  u64 rand_a = base::SourceTrueRandomSeed();
  u64 rand_b = base::SourceTrueRandomSeed();

  // Process fingerprint: PID (+ padding).
  u64 pid = 0;
#ifdef __linux__
  pid = u64(getpid());
#endif

  // 2. Build entropy string.
  char entropy[256];
  int len =
      ::snprintf(entropy, sizeof(entropy), "%llu_%llu_%llu_%llu_%llu",
                 (unsigned long long)ms, (unsigned long long)count,
                 (unsigned long long)rand_a, (unsigned long long)rand_b,
                 (unsigned long long)pid);

  // 3. Hash with SHA-256.
  Sha256Hash hash = Sha256(entropy, mem_size(len));

  // 4. Encode as base36.
  EncodeBase36(hash.bytes, 32, out, kCuid2Length);

  // 5. Force first character to be a letter [a-z].
  //    If it's a digit, remap using the byte value.
  if (out[0] >= '0' && out[0] <= '9') {
    out[0] = 'a' + (hash.bytes[0] % 26);
  }

  out[kCuid2Length] = '\0';
}

bool IsValidCuid2(const char* str) {
  if (!str)
    return false;

  // First character must be a lowercase letter.
  if (str[0] < 'a' || str[0] > 'z')
    return false;

  i32 len = 0;
  for (const char* p = str; *p; ++p, ++len) {
    char c = *p;
    bool is_lower = (c >= 'a' && c <= 'z');
    bool is_digit = (c >= '0' && c <= '9');
    if (!is_lower && !is_digit)
      return false;
  }

  return len == kCuid2Length;
}

}  // namespace base
