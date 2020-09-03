
project("Server")
    language("C#")
    targetname("NODAServer")
    dotnetframework("netcoreapp3.1")
    kind("ConsoleApp")
    files({
        "*.cs"
    })
    includedirs({
        "."
    })