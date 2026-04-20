// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/strings/format.h>

namespace {

template <typename... Args>
base::String F(const char* fmt, const Args&... args) {
  return base::Format(fmt, args...);
}

TEST(Format, PlainText) {
  EXPECT_STREQ(F("hello").c_str(), "hello");
  EXPECT_STREQ(F("").c_str(), "");
}

TEST(Format, EscapedBraces) {
  EXPECT_STREQ(F("{{ }}").c_str(), "{ }");
  EXPECT_STREQ(F("{{{}}}", 1).c_str(), "{1}");
}

TEST(Format, AutoIndex) {
  EXPECT_STREQ(F("{} {} {}", 1, 2, 3).c_str(), "1 2 3");
}

TEST(Format, ExplicitIndex) {
  EXPECT_STREQ(F("{1} {0} {1}", "a", "b").c_str(), "b a b");
}

TEST(Format, MissingArg) {
  EXPECT_STREQ(F("{} {}", 1).c_str(), "1 {?}");
}

TEST(Format, IntegerSigns) {
  EXPECT_STREQ(F("{}", 0).c_str(), "0");
  EXPECT_STREQ(F("{}", 42).c_str(), "42");
  EXPECT_STREQ(F("{}", -42).c_str(), "-42");
  EXPECT_STREQ(F("{:+}", 42).c_str(), "+42");
  EXPECT_STREQ(F("{: }", 42).c_str(), " 42");
  EXPECT_STREQ(F("{:+}", -42).c_str(), "-42");
}

TEST(Format, IntegerINT64MinNoOverflow) {
  i64 v = -9223372036854775807LL - 1;
  EXPECT_STREQ(F("{}", v).c_str(), "-9223372036854775808");
}

TEST(Format, IntegerBases) {
  EXPECT_STREQ(F("{:x}", 255).c_str(), "ff");
  EXPECT_STREQ(F("{:X}", 255).c_str(), "FF");
  EXPECT_STREQ(F("{:#x}", 255).c_str(), "0xff");
  EXPECT_STREQ(F("{:#X}", 255).c_str(), "0XFF");
  EXPECT_STREQ(F("{:o}", 8).c_str(), "10");
  EXPECT_STREQ(F("{:#o}", 8).c_str(), "010");
  EXPECT_STREQ(F("{:b}", 5).c_str(), "101");
  EXPECT_STREQ(F("{:#b}", 5).c_str(), "0b101");
}

TEST(Format, IntegerWidthAlign) {
  EXPECT_STREQ(F("{:5}", 42).c_str(), "   42");
  EXPECT_STREQ(F("{:<5}", 42).c_str(), "42   ");
  EXPECT_STREQ(F("{:>5}", 42).c_str(), "   42");
  EXPECT_STREQ(F("{:^5}", 42).c_str(), " 42  ");
  EXPECT_STREQ(F("{:*^5}", 42).c_str(), "*42**");
  EXPECT_STREQ(F("{:05}", 42).c_str(), "00042");
  EXPECT_STREQ(F("{:05}", -42).c_str(), "-0042");
  EXPECT_STREQ(F("{:#08x}", 255).c_str(), "0x0000ff");
}

TEST(Format, IntegerChar) {
  EXPECT_STREQ(F("{:c}", 65).c_str(), "A");
}

TEST(Format, BoolText) {
  EXPECT_STREQ(F("{}", true).c_str(), "true");
  EXPECT_STREQ(F("{}", false).c_str(), "false");
  EXPECT_STREQ(F("{:d}", true).c_str(), "1");
  EXPECT_STREQ(F("{:>7}", true).c_str(), "   true");
}

TEST(Format, CharFormatting) {
  char ch = 'Z';
  EXPECT_STREQ(F("{}", ch).c_str(), "Z");
  EXPECT_STREQ(F("{:d}", ch).c_str(), "90");
  EXPECT_STREQ(F("{:*<3}", ch).c_str(), "Z**");
}

TEST(Format, StringLiteral) {
  EXPECT_STREQ(F("{}", "world").c_str(), "world");
  EXPECT_STREQ(F("{:>10}", "hi").c_str(), "        hi");
  EXPECT_STREQ(F("{:<10}", "hi").c_str(), "hi        ");
  EXPECT_STREQ(F("{:.3}", "abcdef").c_str(), "abc");
  EXPECT_STREQ(F("{:>6.3}", "abcdef").c_str(), "   abc");
}

TEST(Format, BaseString) {
  base::String s = "hello";
  EXPECT_STREQ(F("[{}]", s).c_str(), "[hello]");
}

TEST(Format, StringRef) {
  base::StringRef sr("ref");
  EXPECT_STREQ(F("[{}]", sr).c_str(), "[ref]");
}

TEST(Format, NullCStr) {
  const char* p = nullptr;
  EXPECT_STREQ(F("{}", p).c_str(), "(null)");
}

TEST(Format, FloatDefault) {
  EXPECT_STREQ(F("{}", 1.5).c_str(), "1.5");
}

TEST(Format, FloatFixedPrecision) {
  EXPECT_STREQ(F("{:.2f}", 3.14159).c_str(), "3.14");
  EXPECT_STREQ(F("{:.0f}", 3.6).c_str(), "4");
  EXPECT_STREQ(F("{:8.2f}", 3.14).c_str(), "    3.14");
  EXPECT_STREQ(F("{:08.2f}", 3.14).c_str(), "00003.14");
}

TEST(Format, FloatScientific) {
  EXPECT_STREQ(F("{:.2e}", 1500.0).c_str(), "1.50e+03");
}

TEST(Format, Pointer) {
  int x = 0;
  base::String s = F("{}", &x);
  ASSERT_GE(s.size(), 3u);
  EXPECT_EQ(s[0], '0');
  EXPECT_EQ(s[1], 'x');
}

TEST(Format, Nullptr) {
  EXPECT_STREQ(F("{}", nullptr).c_str(), "0x0");
}

TEST(Format, MultipleArgsMixed) {
  base::String result =
      F("name={}, age={}, ratio={:.1f}", "vince", 30, 0.875);
  EXPECT_STREQ(result.c_str(), "name=vince, age=30, ratio=0.9");
}

TEST(Format, Utf8FormatString) {
  // The format string itself is UTF-8; bytes pass through verbatim.
  base::String s = F("héllo {} \xE2\x98\x83", "wörld");
  EXPECT_STREQ(s.c_str(), "héllo wörld \xE2\x98\x83");
}

TEST(Format, Char8Argument) {
  const char8_t* greeting = u8"hé";
  EXPECT_STREQ(F("[{}]", greeting).c_str(), "[hé]");
}

TEST(Format, Char8Literal) {
  EXPECT_STREQ(F("[{}]", u8"héllo").c_str(), "[héllo]");
}

TEST(Format, Char16Transcode) {
  const char16_t* greeting = u"héllo";
  EXPECT_STREQ(F("[{}]", greeting).c_str(), "[héllo]");
}

TEST(Format, Char16Surrogates) {
  // U+1F600 GRINNING FACE encoded as surrogate pair D83D DE00.
  const char16_t emoji[] = {0xD83D, 0xDE00, 0};
  EXPECT_STREQ(F("{}", emoji).c_str(), "\xF0\x9F\x98\x80");
}

TEST(Format, Char32Transcode) {
  const char32_t* msg = U"héllo \U0001F600";
  EXPECT_STREQ(F("{}", msg).c_str(), "héllo \xF0\x9F\x98\x80");
}

TEST(Format, WideTranscode) {
  const wchar_t* msg = L"héllo";
  EXPECT_STREQ(F("[{}]", msg).c_str(), "[héllo]");
}

TEST(Format, WideStringLong) {
  // Force the heap fallback for wide transcoding by overflowing the inline
  // 256-byte buffer used by FormatWideString.
  wchar_t buf[260];
  for (int i = 0; i < 259; ++i) buf[i] = L'a';
  buf[259] = 0;
  base::String s = F("{}", buf);
  EXPECT_EQ(s.size(), 259u);
  for (mem_size i = 0; i < s.size(); ++i) EXPECT_EQ(s[i], 'a');
}

TEST(FormatTo, StackBufferReportsTruncation) {
  char buf[8];
  mem_size n = base::FormatTo(buf, sizeof(buf), "{}{}{}", "abcd", "efgh",
                              "ijkl");
  EXPECT_EQ(n, 12u);
  EXPECT_EQ(buf[sizeof(buf) - 1], 0);
  EXPECT_STREQ(buf, "abcdefg");
}

TEST(FormatTo, ZeroBufferIsSafe) {
  char* buf = nullptr;
  mem_size n = base::FormatTo(buf, 0, "should-not-write");
  EXPECT_EQ(n, 0u);
}

TEST(FormatTo, AppendsToExistingString) {
  base::String s = "prefix:";
  base::FormatTo(s, "{}+{}", 1, 2);
  EXPECT_STREQ(s.c_str(), "prefix:1+2");
}

}  // namespace
