-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

filter("configurations:Release")
    defines("Q_DEBUG")

project("x64dbg_plugin")
    kind("SharedLib")
    -- TODO: set targetname based on arch
    -- once we support more than one!
    -- e.g x32dbg
    targetname("x64dbg_RETK")
    include_meta()
    files({
        "**.cpp",
        "**.h",
        "**.ui",
        "**.qrc",
        "Noda.rc",
    })
    links({
        -- ida qt
        "Qt5Core.lib",
        "Qt5Gui.lib",
        "Qt5PrintSupport.lib",
        "Qt5Widgets.lib",

        -- network
        "enet",
        "fmtlib"
    })
    defines({
        -- idaq constants
        "__NT__",
        "__QT__",
    })
    includedirs({
        ".",
        blu.extdir .. "/enet/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
    })
    libdirs({
        blu.extdir .. "/idasdk72/lib/x64_win_vc_64",
        blu.extdir .. "/idasdk72/lib/x64_win_qt",
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