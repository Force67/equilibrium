
project("Server")
    language("C#")
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