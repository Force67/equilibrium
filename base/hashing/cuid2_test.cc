// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/hashing/cuid2.h>
#include <base/hashing/sha256.h>
#include <gtest/gtest.h>

#include <cstring>
#include <set>
#include <string>

// ── SHA-256 ──────────────────────────────────────────────────────────

TEST(Sha256, EmptyStringMatchesNistVector) {
  base::Sha256Hash hash = base::Sha256("", 0);
  // NIST test vector: SHA-256("") =
  //   e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855
  static const u8 expected[32] = {
      0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
      0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
      0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
  EXPECT_EQ(std::memcmp(hash.bytes, expected, 32), 0);
}

TEST(Sha256, AbcMatchesNistVector) {
  base::Sha256Hash hash = base::Sha256("abc");
  // SHA-256("abc") =
  //   ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad
  static const u8 expected[32] = {
      0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40,
      0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17,
      0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
  EXPECT_EQ(std::memcmp(hash.bytes, expected, 32), 0);
}

TEST(Sha256, DeterministicOutput) {
  const char* input = "deterministic test input 12345";
  base::Sha256Hash a = base::Sha256(input);
  base::Sha256Hash b = base::Sha256(input);
  EXPECT_EQ(std::memcmp(a.bytes, b.bytes, 32), 0);
}

// ── CUID2 ────────────────────────────────────────────────────────────

TEST(Cuid2, GenerateValidFormat) {
  char id[base::kCuid2Length + 1];
  base::GenerateCuid2(id);

  // Must be exactly kCuid2Length characters.
  EXPECT_EQ(std::strlen(id), size_t(base::kCuid2Length));

  // First character must be a letter.
  EXPECT_GE(id[0], 'a');
  EXPECT_LE(id[0], 'z');

  // All characters must be [a-z0-9].
  for (int i = 0; i < base::kCuid2Length; ++i) {
    bool ok = (id[i] >= 'a' && id[i] <= 'z') || (id[i] >= '0' && id[i] <= '9');
    EXPECT_TRUE(ok) << "Invalid char at index " << i << ": " << id[i];
  }
}

TEST(Cuid2, UniqueAcross1000Calls) {
  std::set<std::string> ids;
  for (int i = 0; i < 1000; ++i) {
    char id[base::kCuid2Length + 1];
    base::GenerateCuid2(id);
    ids.insert(std::string(id));
  }
  EXPECT_EQ(ids.size(), 1000u);
}

TEST(Cuid2, IsValidCuid2AcceptsValid) {
  char id[base::kCuid2Length + 1];
  base::GenerateCuid2(id);
  EXPECT_TRUE(base::IsValidCuid2(id));
}

TEST(Cuid2, IsValidCuid2RejectsInvalid) {
  EXPECT_FALSE(base::IsValidCuid2(nullptr));
  EXPECT_FALSE(base::IsValidCuid2(""));
  EXPECT_FALSE(base::IsValidCuid2("abc"));                          // too short
  EXPECT_FALSE(base::IsValidCuid2("abcdefghijklmnopqrstuvwxyz"));   // too long (26)
  EXPECT_FALSE(base::IsValidCuid2("0bcdefghijklmnopqrstuvwx"));     // starts with digit
  EXPECT_FALSE(base::IsValidCuid2("Abcdefghijklmnopqrstuvwx"));     // starts with uppercase
  EXPECT_FALSE(base::IsValidCuid2("abcdefghijkl!nopqrstuvwx"));     // special char
}

TEST(Cuid2, FusedConstantIsValid) {
  EXPECT_TRUE(base::IsValidCuid2(base::kFusedTestCuid2));
}

TEST(Cuid2, GeneratedIdIsNeverFusedConstant) {
  for (int i = 0; i < 100; ++i) {
    char id[base::kCuid2Length + 1];
    base::GenerateCuid2(id);
    EXPECT_STRNE(id, base::kFusedTestCuid2);
  }
}
