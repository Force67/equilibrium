
project("LegacyServer")
    language("C#")
    location(blu.netout)
    objdir(blu.netdir)
    dotnetframework("netcoreapp3.1")
    targetname("NODAServer")
    kind("ConsoleApp")
    icon("logo.ico")
    files({
        "**.cs"
    })
    links({
        "FlatBuffers"
    })
    nuget({
        "Discord.Net:2.2.0",
        "System.Data.SQLite.Core:1.0.113.1"
    })
    -- auto deploy the native dll
    postbuildcommands({
        --"{COPY} ../ENetNative_64.dll ENetNative_64.dll"
        "echo f | xcopy /Y ..\\\\ENetNative_64.dll ."
    })