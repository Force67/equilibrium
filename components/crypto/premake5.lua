-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

component("crypto")
  files({
    "*.cc",
    "*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})

unittest("crypto:crypto_tests")
  files({
    "*.cc",
    "*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})
  add_generic_test_main()