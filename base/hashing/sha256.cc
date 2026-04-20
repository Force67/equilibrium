// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// SHA-256 implementation based on the reference from FIPS 180-4.
// Public domain.

#include "sha256.h"

#include <cstring>

namespace {

static const u32 kK[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

inline u32 Rotr(u32 x, int n) {
  return (x >> n) | (x << (32 - n));
}

inline u32 Ch(u32 x, u32 y, u32 z) {
  return (x & y) ^ (~x & z);
}

inline u32 Maj(u32 x, u32 y, u32 z) {
  return (x & y) ^ (x & z) ^ (y & z);
}

inline u32 Sigma0(u32 x) {
  return Rotr(x, 2) ^ Rotr(x, 13) ^ Rotr(x, 22);
}

inline u32 Sigma1(u32 x) {
  return Rotr(x, 6) ^ Rotr(x, 11) ^ Rotr(x, 25);
}

inline u32 sigma0(u32 x) {
  return Rotr(x, 7) ^ Rotr(x, 18) ^ (x >> 3);
}

inline u32 sigma1(u32 x) {
  return Rotr(x, 17) ^ Rotr(x, 19) ^ (x >> 10);
}

struct Sha256Ctx {
  u32 state[8];
  u64 count;
  u8 buffer[64];

  Sha256Ctx() {
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;
    count = 0;
  }

  void Transform(const u8 block[64]) {
    u32 w[64];
    for (int i = 0; i < 16; ++i) {
      w[i] = (u32(block[i * 4]) << 24) | (u32(block[i * 4 + 1]) << 16) |
             (u32(block[i * 4 + 2]) << 8) | u32(block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
      w[i] = sigma1(w[i - 2]) + w[i - 7] + sigma0(w[i - 15]) + w[i - 16];
    }

    u32 a = state[0], b = state[1], c = state[2], d = state[3];
    u32 e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 64; ++i) {
      u32 t1 = h + Sigma1(e) + Ch(e, f, g) + kK[i] + w[i];
      u32 t2 = Sigma0(a) + Maj(a, b, c);
      h = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
  }

  void Update(const u8* data, mem_size len) {
    mem_size index = count % 64;
    count += len;

    mem_size i = 0;
    if (index) {
      mem_size part = 64 - index;
      if (len >= part) {
        ::memcpy(buffer + index, data, part);
        Transform(buffer);
        i = part;
      } else {
        ::memcpy(buffer + index, data, len);
        return;
      }
    }

    for (; i + 64 <= len; i += 64) {
      Transform(data + i);
    }

    if (i < len) {
      ::memcpy(buffer, data + i, len - i);
    }
  }

  base::Sha256Hash Finalize() {
    u64 bits = count * 8;
    mem_size index = count % 64;

    u8 pad = (index < 56) ? u8(56 - index) : u8(120 - index);
    u8 padding[128] = {0x80};
    Update(padding, pad);

    u8 bits_be[8];
    for (int i = 0; i < 8; ++i)
      bits_be[i] = u8(bits >> (56 - i * 8));
    Update(bits_be, 8);

    base::Sha256Hash hash;
    for (int i = 0; i < 8; ++i) {
      hash.bytes[i * 4] = u8(state[i] >> 24);
      hash.bytes[i * 4 + 1] = u8(state[i] >> 16);
      hash.bytes[i * 4 + 2] = u8(state[i] >> 8);
      hash.bytes[i * 4 + 3] = u8(state[i]);
    }
    return hash;
  }
};

}  // namespace

namespace base {

Sha256Hash Sha256(const void* data, mem_size size) {
  Sha256Ctx ctx;
  ctx.Update(reinterpret_cast<const u8*>(data), size);
  return ctx.Finalize();
}

Sha256Hash Sha256(const char* str) {
  return Sha256(str, ::strlen(str));
}

}  // namespace base
