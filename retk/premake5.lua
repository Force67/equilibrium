-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

push_scope("RETK")
include("./components")

scope_group("App")
include("main")
include("sync_server/dedi_server")
include("sync_server/server")

scope_group("Integrations")
include("integrations/ida")
include("integrations/x64dbg")