
project("Server")
    language("C#")
    location(blu.netout)
    objdir(blu.netdir)
    dotnetframework("netcoreapp3.1")
    targetname("NODAServer")
    kind("ConsoleApp")
    files({
        "**.cs"
    })
    links({
        "FlatBuffers"
    })
    nuget({
        "Discord.Net:2.2.0",
        "Npgsql:4.1.4"
    })
    -- auto deploy the native dll
    postbuildcommands({
        --"{COPY} ../ENetNative_64.dll ENetNative_64.dll"
        "echo f | xcopy /Y ..\\\\ENetNative_64.dll ."
    })