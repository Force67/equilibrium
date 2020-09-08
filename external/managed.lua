
project("Flatbuffers")
    language("C#")
    kind("SharedLib")
    objdir(blu.netdir)
    dotnetframework("netcoreapp3.1")
    includedirs({
        "flatbuffers/net"
    })
    files({
        "flatbuffers/net/Flatbuffers/*.cs"
    })