-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

push_scope("NEMISIS")
--include("./components")

scope_group("App")
include("runner")

scope_group("SDK")
include("sdk")
include("sample_apps")