-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

component("entitlement")
  files({
    "keystone/*.cc",
    "keystone/*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})

component("entitlement:issuing_tool")
  files({
    "issuing_tool/*.cc",
    "issuing_tool/*.h",
  })
  links("entitlement")
  dependencies({
    "fmtlib", 
    "mbedtls"})

component("entitlement:licensee_database_server")
  files({
    "licensee_database_server/*.cc",
    "licensee_database_server/*.h",
  })
  links("entitlement")
  dependencies({
    "fmtlib", 
    "mbedtls"})

unittest("entitlement:entilement_test")
  files({
    "keystone/*.cc",
    "keystone/*.h",
  })
  dependencies({
    "fmtlib", 
    "mbedtls"})
  add_generic_test_main()