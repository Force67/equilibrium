-- Copyright (c) NOMAD Group<nomad-group.net>

-- mock flatbuffer files
os.rmdir("common/moc_protocol")
os.mkdir("common/moc_protocol")
matches = os.matchfiles("common/protocol/**.fbs")
for k,v in pairs(matches) do 
    local fb_dir = path.getabsolute("../tools/build")
    print(v)
    os.executef("%s --cpp -o common/moc_protocol %s", fb_dir .. "/flatc", v) 
end

function include_netlib()
    
end

-- Legacy component: include("legacy_server")
group("Server")
include("sync_server/dedicated_main")
include("sync_server/server")
include("sync_server/tests/test_client")

group("Plugins")
include("plugins/idaplugin")
include("plugins/xdbgplugin")

group("Shared")
include("common")