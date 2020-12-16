-- Copyright (c) NOMAD Group<nomad-group.net>

-- mock flatbuffer files
os.rmdir("common/protocol/generated")
os.mkdir("common/protocol/generated")
matches = os.matchfiles("common/protocol/**.fbs")
for k,v in pairs(matches) do 
    local fb_dir = path.getabsolute("../tools/build")
    print(v)
    os.executef("%s --cpp -o common/protocol/generated %s", fb_dir .. "/flatc", v) 
end

function include_netlib()
    
end

-- Legacy component: include("legacy_server")
group("Server")
include("sync_server/dedi_server")
include("sync_server/server")

group("Plugins")
include("plugins/ida_plugin")
include("plugins/old_idaplugin")
include("plugins/xdbgplugin")

group("Shared")
include("common")