-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

include("eq/crypto")
include("eq/database")
include("eq/entitlement")
include("eq/gpu")
include("eq/network")
--include("eq/ui")

-- we intentionally don't include the eq dir here, as the user should
-- be forced to explicitly include it
function include_eq_components()
  includedirs(blu.rootdir .. "/components")
end