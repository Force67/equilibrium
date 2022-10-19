-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

component2("eq_crypto")
  files({
    "*.cc",
    "*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})

unittest2("eq_crypto:crypto_tests")
  files({
    "*.cc",
    "*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})
  add_generic_test_main()