-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

include("crypto")
include("database")
include("entitlement")
include("gpu")
include("network")
include("ui")

function include_eq_components()
  includedirs(blu.rootdir .. "/components")
end