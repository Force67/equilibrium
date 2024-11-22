// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>

#if defined(__SSE4_2__) && (defined(__GNUC__) || defined(__clang__))
#include <nmmintrin.h>  // For _mm_crc32_u8 (SSE4.2)
#elif defined(_M_X64) && defined(_MSC_VER)
#include <intrin.h>  // For __cpuid, __crc32b (MSVC)
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#include <base/arch.h>

namespace base {
inline u32 CRC32_SW(const byte* data,
                    mem_size length,
                    const u32 polynomial = 0xEDB88320, /*Standard CRC-32 polynomial*/
                    const u32 prev_crc = 0xFFFFFFFF) {
  u32 crc = prev_crc;

  for (mem_size i = 0; i < length; ++i) {
    byte b = data[i];
    crc ^= b;

    for (byte j = 0; j < 8; ++j) {
      if (crc & 1)
        crc = (crc >> 1) ^ polynomial;
      else
        crc >>= 1;
    }
  }

  return ~crc;
}

inline u32 CRC32C_SW(
    const byte* data,
    mem_size length,
    const u32 polynomial = 0x82f63b78, /*Bit-reflected CRC-32C polynomial*/
                                       // also known as the Castagnoli poly:
    // https://en.wikipedia.org/wiki/Cyclic_redundancy_check
    u32 prev_crc = 0xFFFFFFFF) {
  u32 crc = ~prev_crc;  // this is the difference to above

  for (mem_size i = 0; i < length; ++i) {
    byte b = data[i];
    crc ^= b;

    for (byte j = 0; j < 8; ++j) {
      if (crc & 1)
        crc = (crc >> 1) ^ polynomial;
      else
        crc >>= 1;
    }
  }

  return ~crc;
};

u32 CRC32C(const byte* data, mem_size length, u32 previousCrc32 = 0xFFFFFFFF) {
  // virtually all 64 bit chips have it
#ifdef ARCH_X86_64
  uint32_t crc = ~previousCrc32;

  // The Intel instruction uses the iSCSI (Castagnoli) polynomial, for the CRC
  // often referred to as CRC-32C.
  // this differs from the "known" way.
  // The table is for the Ethernet/ZIP/etc. CRC, often referred to as CRC-32.
  for (std::size_t i = 0; i < length; ++i) {
    crc = _mm_crc32_u8(crc, data[i]);
  }

  return ~crc;
#elif defined(__ARM_NEON)
// Fallback to a NEON-optimized software version or use a library like
// "crc32_arm" Note that ARM NEON doesn't have a built-in CRC32 instruction.
#error Add neon support.
#else
  return CRC32C_SW(data, length, previousCrc32);
#endif
}
}  // namespace base