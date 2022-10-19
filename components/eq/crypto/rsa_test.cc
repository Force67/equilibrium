// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/random.h>

#include "rsa.h"	
#include "mbed_raii_wrappers.h"	// for mbed::RsaKey

namespace {
constexpr char kPublicKey[] = R"(
-----BEGIN RSA PRIVATE KEY-----
MIICWgIBAAKBgEVvZEuRbruyUNIvzasdiXxZBDeY+shV599/6lclztEqcBoaDkxZ
AWlQbSh5llVMBac2HNeM/DaaJ4wZVfktcOmDIaLgR8iLOUPo1LYsGLQzPv7u75A3
oPP53mqmtokx6pYH22A2AiINKmKJCW838S9cZpStbv0U2AzeJflQJq+vAgMBAAEC
gYAI/aF4/33dI/u/G51/+LUiuswG/vIKGsWJdSC85BJhlyMo3hLX4/aA0n8v7H6A
0PUyb5lQd9r4nwFb7LPrLe0ezOfa7XF36Az4qW9s7BUUG0WutnS3U3wHS+QpKd5w
VK17TItKj+s8BhW6sWb8bf72x6BkeKqISngRDREr06lYMQJBAIjvPgEeErGNkuza
8jhVBiSj5pmgE+6gdE9k9h9Rsda5LyPS91fDD6aqPD88AVcYouO6uXK8BFfrrAY2
4mmoZQUCQQCBzzV4R8bRnqM3OL56Ak01lqF4QvFEnfLY4179DvqeSnEGtR1K6k2r
aQSWYHhJXSQTF7tXNQY5BV8PQQ6rc2AjAkALO5Tagp0F85cZ1twHwvG+D7Pa5ka8
TG0+Fgoah5kPBCHZznR7voSKJ2zJwyJZAqxaAfc0cjfo/mweFmJ8SFQBAkBXdDto
2qqnJQJ/PLqPRqev19jXRfp4KRWoa57eTM4qAKsfQrwGcc58EhemHD5N5vNb7Zwo
h6+2kNjXVRRGDEgTAkAxfZ+qXTmnqhiH1RG0pMoyHbtQLd66sF8ZPoiA6l8ciK4V
PzRDlg/UHM5lZAAQCV7qR76DJpOhtN4p4NqiIEa2
-----END RSA PRIVATE KEY-----)";

TEST(RSA, SignVerify) {
  byte data[]{0, 1, 2, 3, 4, 5};

  crypto::PublicKeyWrap pk;
  const base::Span s(reinterpret_cast<const byte*>(&kPublicKey[0]),
                     sizeof(kPublicKey));

  crypto::EntropyWrap entropy;
  crypto::CrtDrbgWrap random;

  u64 entropy_data[8]{};
  for (auto i = 0; i < 8; i++) {
    entropy_data[i] = base::RandomUint() + base::RandomUint();
  }

  // seed the random number generator
  bool b = random.Seed(entropy, base::Span(reinterpret_cast<byte*>(&entropy_data[0]),
                                           sizeof(u64) * 8));
  EXPECT_TRUE(b);

  EXPECT_TRUE(pk.ParsePrivateKey(s, random));

  //mbedtls_parse_key

  base::Vector<byte> signage;
  EXPECT_TRUE(crypto::RSASign(pk, entropy, random, data, signage));

  EXPECT_TRUE(crypto::RSAVerify(pk, data, signage));

  fmt::print("The following fails on purpose\n");
  // atlter data
  // the error in the console is expected
  data[0] = 9;
  EXPECT_FALSE(crypto::RSAVerify(pk, data, signage));
}
}  // namespace
