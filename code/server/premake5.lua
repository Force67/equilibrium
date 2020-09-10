
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
        "Discord.Net",
    })
    nuget({
        "Discord.Net:2.2.0"
    })