-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

local function build_entitlement()
  files({
    "*.cc",
    "*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})
end

component("entitlement")
  build_entitlement()

unittest("entitlement:entilement_test")
  build_entitlement()
  add_generic_test_main()