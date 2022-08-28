-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

component("entitlement")
  files({
    "keystone/*.cc",
    "keystone/*.h",
    "protocol/license.fbs",
  })
  includedirs(blu.extdir .. "/flatbuffers/include")
  dependencies({
    "fmtlib", 
    "mbedtls"})

unittest("entitlement:entilement_test")
  files({
    "keystone/*.cc",
    "keystone/*.h"
  })
  includedirs(blu.extdir .. "/flatbuffers/include")
  dependencies({
    "fmtlib", 
    "mbedtls"})
  add_generic_test_main()

component("entitlement:issuing_tool")
  kind("ConsoleApp")
  rules("CompileFlatbuffers")
  files({
    "issuing_tool/*.cc",
    "issuing_tool/*.h",
    "protocol/license.fbs",
  })
  includedirs(blu.extdir .. "/flatbuffers/include")
  links({"entitlement", "crypto"})
  dependencies({
    "fmtlib", 
    "mbedtls"})

unittest("entitlement:issuing_tool:issuing_test")
  files({
    "issuing_tool/*.cc",
    "issuing_tool/*.h",
  })
  links({"entitlement", "crypto"})
  dependencies({
    "fmtlib", 
    "mbedtls"})
  add_generic_test_main()

component("entitlement:licensee_database_server")
  files({
    "licensee_database_server/*.cc",
    "licensee_database_server/*.h",
  })
  links({"entitlement", "crypto"})
  dependencies({
    "fmtlib", 
    "mbedtls"})