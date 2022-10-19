// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "base64.h"

namespace {
struct Complex {
  int a, b, c;
};

constexpr char kTestText[] =
    "There are many variations of passages of Lorem Ipsum available, but the "
    "majority have suffered alteration in some form, by injected humour, or "
    "randomised words which don't look even slightly believable. If you are going "
    "to use a passage of Lorem Ipsum, you need to be sure there isn't anything "
    "embarrassing hidden in the middle of text. All the Lorem Ipsum generators on "
    "the Internet tend to repeat predefined chunks as necessary, making this the "
    "first true generator on the Internet. It uses a dictionary of over 200 Latin "
    "words, combined with a handful of model sentence structures, to generate Lorem "
    "Ipsum which looks reasonable. The generated Lorem Ipsum is therefore always "
    "free from repetition, injected humour, or non-characteristic words etc.";

TEST(Crypto, Base64EncodeDecodeSimple) {
  const Complex c{.a = 0xBABE, .b = 0x1337, .c = 0xC0FFE};

  mem_size out_length = 0;
  base::String result;

  base::Span span(reinterpret_cast<const byte*>(&c), sizeof(c));
  EXPECT_TRUE(crypto::Base64Encode(span, result));

  base::String decode_result; // is a string, that could contain binary data though
  EXPECT_TRUE(crypto::Base64Decode(result, decode_result));

  EXPECT_TRUE(memcmp(&c, decode_result.data(), sizeof(c)) == 0);
}

TEST(Crypto, Base64EncodeDecodeText) {
  base::String result;
  // encode case where we have to resize
  base::Span span2(reinterpret_cast<const byte*>(&kTestText), sizeof(kTestText));
  EXPECT_TRUE(crypto::Base64Encode(span2, result));

  base::String decode_result;
  EXPECT_TRUE(crypto::Base64Decode(result, decode_result));

  EXPECT_STREQ(kTestText, decode_result.c_str());
}
}  // namespace
