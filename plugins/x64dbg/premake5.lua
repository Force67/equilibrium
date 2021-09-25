-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

project("x64dbg_plugin")
    kind("SharedLib")
    -- TODO: set targetname based on arch
    -- once we support more than one!
    -- e.g x32dbg
    targetname("x64dbg_RETK")
    include_meta()
    files({
        "**.cc",
        "**.h",
        "**.ui",
        "**.qrc",
        "plugin.rc",
    })
    includedirs({
        ".",
        "../../",
        blu.extdir .. "/enet/include",
        blu.extdir .. "/x64dbgsdk"
    })
    links({
        "database",
        "base",
    })
    defines({
        "BASE_IMPLEMENTATION"
    })
    qt.enable()
    qtprefix("Qt5")
    qtgenerateddir("ui/moc")
    -- TODO: think about a better way of doing this
    qtpath("C:/Qt/Qt5.6.3/5.6.3/msvc2015_64")
    qtmodules({
        -- we have to restrict ourself to this feature set
        -- since ida only ships with these by default
        "core",
        "gui",
        "widgets",
    })