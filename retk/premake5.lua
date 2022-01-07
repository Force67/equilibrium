-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

push_scope("RETK")
include("loader")

scope_group("App")
include("main")
include("sync_server/dedi_server")
include("sync_server/server")

scope_group("Integrations")
include("integrations/ida")
include("integrations/x64dbg")