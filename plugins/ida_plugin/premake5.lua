-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

filter("configurations:Release")
    defines("Q_DEBUG")

project("ida_plugin")
    kind("SharedLib")
    targetname("IDA_Retk76")
    include_meta()
    debugdir("C:\\Users\\vince\\AppData\\Roaming\\IDAPro\\IDA 7.6 SP1")
    debugcommand("C:\\Users\\vince\\AppData\\Roaming\\IDAPro\\IDA 7.6 SP1\\ida64.exe")
    files({
        "**.cc",
        "**.h",
        "ui/**.ui",
        "resources.qrc",
        "plugin.rc",
        "premake5.lua"
    })
    pchheader("pch.h")
    pchsource("pch.cc")
    forceincludes("pch.h")
    links({
        -- ida libraries
        "ida",
        "pro",
        "compress",

        -- ida qt
        "Qt5Core.lib",
        "Qt5Gui.lib",
        "Qt5PrintSupport.lib",
        "Qt5Widgets.lib",
        
        -- third party
        "network",
        "fmtlib",

        "network",
        "database",
        "base",
        "sync"
    })
    -- configure IDA SDK
    defines({
        "__NT__",            -- < Windows NT Target
        "__QT__",            -- < We are building with QT
        --"__UI__",          -- < Define ida types as QT types
        "__EA64__",          -- < Use 64 Bit addressing (Even ida 32 is 64 bit nowadays)
        "NO_OBSOLETE_FUNCS", -- < No depricated functions (Target for latest IDA)

        -- qt symbol workaround, read more here:
        -- https://www.hex-rays.com/blog/ida-qt-under-the-hood/
        "QT_NAMESPACE=QT",
    })
    includedirs({
        ".",
        "../../",
        "../../retk",
        blu.extdir .. "/idasdk75/include",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/xenium"
    })
    libdirs({
        blu.extdir .. "/idasdk75/lib/x64_win_vc_64",
        blu.extdir .. "/idasdk75/lib/x64_win_qt",
    })
    qt.enable()
    qtprefix("Qt5")
    qtgenerateddir("ui/generated")
    -- TODO: think about a better way of doing this
    qtpath("C:/Qt/Qt5.6.3/5.6.3/msvc2015_64")
    qtmodules({
        -- we have to restrict ourself to this feature set
        -- since ida only ships with these by default
        "core",
        "gui",
        "widgets",
    })