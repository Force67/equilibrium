// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/text/code_convert.h>
#include <build/build_config.h>

namespace {

// Convenience: cast a UTF-8 byte literal so EXPECT_EQ on StringU8/StringU16
// works against a fresh literal without sprinkling reinterpret_cast.
inline base::StringU8 U8(const char* s, base::StringU8::size_type n) {
  return base::StringU8(reinterpret_cast<const char8_t*>(s), n);
}
inline base::StringU8 U8(const char* s) {
  base::StringU8::size_type n = 0;
  while (s[n]) ++n;
  return U8(s, n);
}

// UTF-8 -> UTF-16 -----------------------------------------------------------

TEST(UTF8ToUTF16, Empty) {
  base::StringU16 out;
  EXPECT_TRUE(base::UTF8ToUTF16("", 0, &out));
  EXPECT_EQ(out.size(), 0u);
  EXPECT_EQ(base::UTF8ToUTF16(base::StringRef("")).size(), 0u);
}

TEST(UTF8ToUTF16, Ascii) {
  base::StringU16 out = base::UTF8ToUTF16(base::StringRef("hello"));
  ASSERT_EQ(out.size(), 5u);
  EXPECT_EQ(out[0], u'h');
  EXPECT_EQ(out[4], u'o');
}

TEST(UTF8ToUTF16, BMP) {
  // U+00E9 'é' (2 UTF-8 bytes), U+4E16 '世' (3 UTF-8 bytes).
  const char src[] = "\xC3\xA9\xE4\xB8\x96";
  base::StringU16 out = base::UTF8ToUTF16(base::StringRef(src, sizeof(src) - 1));
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0], 0x00E9);
  EXPECT_EQ(out[1], 0x4E16);
}

TEST(UTF8ToUTF16, Supplementary) {
  // U+1F600 GRINNING FACE -> 4-byte UTF-8, encoded as a UTF-16 surrogate pair.
  const char src[] = "\xF0\x9F\x98\x80";
  base::StringU16 out = base::UTF8ToUTF16(base::StringRef(src, 4));
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0], 0xD83D);
  EXPECT_EQ(out[1], 0xDE00);
}

TEST(UTF8ToUTF16, MalformedReplaced) {
  // 0xC3 starts a 2-byte sequence but the next byte is ASCII '!', so the
  // codepoint is malformed and should be replaced with U+FFFD.
  const char src[] = "\xC3!";
  base::StringU16 out;
  EXPECT_FALSE(base::UTF8ToUTF16(src, 2, &out));
  ASSERT_GE(out.size(), 1u);
  EXPECT_EQ(out[0], 0xFFFD);
}

TEST(UTF8ToUTF16, OverwritesPriorContents) {
  base::StringU16 out = u"junk";
  EXPECT_TRUE(base::UTF8ToUTF16("hi", 2, &out));
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0], u'h');
  EXPECT_EQ(out[1], u'i');
}

// UTF-16 -> UTF-8 -----------------------------------------------------------

TEST(UTF16ToUTF8, Empty) {
  base::StringU8 out;
  EXPECT_TRUE(base::UTF16ToUTF8(u"", 0, &out));
  EXPECT_EQ(out.size(), 0u);
}

TEST(UTF16ToUTF8, Ascii) {
  base::StringU8 out = base::UTF16ToUTF8(base::StringRefU16(u"hello", 5));
  EXPECT_EQ(out, U8("hello"));
}

TEST(UTF16ToUTF8, BMP) {
  const char16_t src[] = {0x00E9, 0x4E16};
  base::StringU8 out = base::UTF16ToUTF8(base::StringRefU16(src, 2));
  EXPECT_EQ(out, U8("\xC3\xA9\xE4\xB8\x96"));
}

TEST(UTF16ToUTF8, Surrogates) {
  const char16_t src[] = {0xD83D, 0xDE00};
  base::StringU8 out = base::UTF16ToUTF8(base::StringRefU16(src, 2));
  EXPECT_EQ(out, U8("\xF0\x9F\x98\x80"));
}

TEST(UTF16ToUTF8, UnpairedSurrogateReplaced) {
  // Lone low surrogate is invalid; must be replaced with U+FFFD.
  const char16_t src[] = {0xD83D, u'A'};
  base::StringU8 out;
  EXPECT_FALSE(base::UTF16ToUTF8(src, 2, &out));
  // U+FFFD encoded as UTF-8 is EF BF BD, then 'A'.
  EXPECT_EQ(out, U8("\xEF\xBF\xBD" "A"));
}

// UTF-8 <-> wide (round-trip on whatever wchar_t this platform uses) -------

TEST(WideRoundtrip, Ascii) {
  const char* original = "hello, world";
  base::StringW wide = base::UTF8ToWide(base::StringRefU8(
      reinterpret_cast<const char8_t*>(original), 12));
  ASSERT_EQ(wide.size(), 12u);
  base::StringU8 back = base::WideToUTF8(base::StringRefW(wide.c_str()));
  EXPECT_EQ(back, U8("hello, world"));
}

TEST(WideRoundtrip, BMP) {
  base::StringW wide = base::UTF8ToWide(base::StringRefU8(u8"héllo 世界"));
  base::StringU8 back = base::WideToUTF8(base::StringRefW(wide.c_str()));
  EXPECT_EQ(back, U8("h\xC3\xA9llo \xE4\xB8\x96\xE7\x95\x8C"));
}

TEST(WideRoundtrip, Supplementary) {
  // U+1F600 GRINNING FACE.
  base::StringW wide = base::UTF8ToWide(
      base::StringRefU8(u8"\U0001F600"));
  base::StringU8 back = base::WideToUTF8(base::StringRefW(wide.c_str()));
  EXPECT_EQ(back, U8("\xF0\x9F\x98\x80"));
}

TEST(UTF8ToWide, Char8Overload) {
  base::StringW out;
  EXPECT_TRUE(base::UTF8ToWide(reinterpret_cast<const char8_t*>("héllo"), 6,
                               &out));
  base::StringU8 back = base::WideToUTF8(base::StringRefW(out.c_str()));
  EXPECT_EQ(back, U8("héllo"));
}

TEST(UTF8ToWide, MalformedSignalsFailure) {
  base::StringW out;
  EXPECT_FALSE(base::UTF8ToWide("\xC3!", 2, &out));
  ASSERT_GE(out.size(), 1u);
  EXPECT_EQ(static_cast<u32>(out[0]), 0xFFFDu);
}

// Wide <-> UTF-16 ----------------------------------------------------------

TEST(WideUtf16Roundtrip, BMP) {
  base::StringW wide = base::UTF8ToWide(base::StringRefU8(u8"héllo 世界"));
  base::StringU16 utf16 = base::WideToUTF16(base::StringRefW(wide.c_str()));
  base::StringW wide_back = base::UTF16ToWide(
      base::StringRefU16(utf16.data(), utf16.size()));
  ASSERT_EQ(wide.size(), wide_back.size());
  for (mem_size i = 0; i < wide.size(); ++i) EXPECT_EQ(wide[i], wide_back[i]);
}

TEST(WideUtf16Roundtrip, Supplementary) {
  // Round-trips a non-BMP codepoint through the wide<->UTF-16 transcoders.
  // On UTF-32 wchar_t the wide string holds a single char, but the UTF-16
  // intermediate is a surrogate pair.
  base::StringW wide =
      base::UTF8ToWide(base::StringRefU8(u8"\U0001F600"));
  base::StringU16 utf16 = base::WideToUTF16(base::StringRefW(wide.c_str()));
  ASSERT_EQ(utf16.size(), 2u);
  EXPECT_EQ(utf16[0], 0xD83D);
  EXPECT_EQ(utf16[1], 0xDE00);
  base::StringW wide_back = base::UTF16ToWide(
      base::StringRefU16(utf16.data(), utf16.size()));
  ASSERT_EQ(wide.size(), wide_back.size());
  for (mem_size i = 0; i < wide.size(); ++i) EXPECT_EQ(wide[i], wide_back[i]);
}

TEST(WideUtf16Roundtrip, Empty) {
  base::StringU16 u16 = base::WideToUTF16(base::StringRefW(L""));
  EXPECT_EQ(u16.size(), 0u);
  base::StringW back = base::UTF16ToWide(base::StringRefU16(u"", 0));
  EXPECT_EQ(back.size(), 0u);
}

// ASCII helpers -------------------------------------------------------------

TEST(ASCIIToWide, Basic) {
  base::StringW w = base::ASCIIToWide(base::StringRef("hello"));
  ASSERT_EQ(w.size(), 5u);
  EXPECT_EQ(w[0], L'h');
  EXPECT_EQ(w[4], L'o');
}

TEST(ASCIIToWide, EmptyAndHighBit) {
  EXPECT_EQ(base::ASCIIToWide(base::StringRef("")).size(), 0u);
  base::StringW w = base::ASCIIToWide(base::StringRef("\x7F"));
  ASSERT_EQ(w.size(), 1u);
  EXPECT_EQ(static_cast<u32>(w[0]), 0x7Fu);
}

TEST(WideToASCII, Basic) {
  base::String s = base::WideToASCII(base::StringRefW(L"hello"));
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST(WideToASCII, Empty) {
  EXPECT_EQ(base::WideToASCII(base::StringRefW(L"")).size(), 0u);
}

TEST(ASCIIRoundtrip, Roundtrip) {
  const char* src = "Round-trip 1234!";
  base::StringW w = base::ASCIIToWide(base::StringRef(src));
  base::String back = base::WideToASCII(base::StringRefW(w.c_str()));
  EXPECT_STREQ(back.c_str(), src);
}

}  // namespace
