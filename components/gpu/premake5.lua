-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("gpu")
  dependencies("fmtlib")
  files({
    "**.cc",
    "**.c",
    "**.h",
    "**.inl"})