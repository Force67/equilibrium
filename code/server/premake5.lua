
project("Server")
    language("C#")
    location(blu.netout)
    objdir(blu.netdir)
    targetname("NODAServer")
    dotnetframework("netcoreapp3.1")
    kind("ConsoleApp")
    files({
        "**.cs"
    })
    links({
        "FlatBuffers",
    })
    includedirs({
        "."
    })