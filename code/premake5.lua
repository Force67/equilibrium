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

-- Legacy component: include("legacy_server")
include("sync_server/dedicated_main")
include("sync_server/server")
include("common")
include("ida_plugin")
include("x64dbg_plugin")