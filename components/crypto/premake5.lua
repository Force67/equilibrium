-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

component2("crypto")
  files({
    "*.cc",
    "*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})

unittest2("crypto:crypto_tests")
  files({
    "*.cc",
    "*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})
  add_generic_test_main()