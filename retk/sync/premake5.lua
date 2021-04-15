-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Components")

os.rmdir("protocol/generated")
os.mkdir("protocol/generated")
matches = os.matchfiles("protocol/**.fbs")
for k,v in pairs(matches) do 
    local fb_dir = path.getabsolute("../../build")
    print(v)
    os.executef("%s --cpp -o protocol/generated %s", fb_dir .. "/flatc", v) 
end

group("retk/sync")
project("sync")
    kind("StaticLib")
    files({
        "**.cc",
        "**.h",
    })
    links({
        "database",
        "network"
    })
    includedirs({
        ".",
        "../",
        "../../",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/xenium",
    })