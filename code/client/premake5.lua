-- Copyright (c) NOMAD Group<nomad-group.net>

project("NODA - client")
    kind("SharedLib")
    targetname("Noda_72")
    files({
        "**.cpp",
        "**.h",
        "**.ui",
        "**.qrc"
    })
    links({
        -- ida libraries
        "ida",
        "pro",
        "compress",

        -- ida qt
        "Qt5Core.lib",
        "Qt5Gui.lib",
        "Qt5PrintSupport.lib",
        "Qt5Widgets.lib"
    })
    defines({
        -- idaq constants
        "__NT__",
        "__UI__",
        "__QT__",

        -- qt symbol workaround, read more here:
        -- https://www.hex-rays.com/blog/ida-qt-under-the-hood/
        "QT_NAMESPACE=QT",

    })
    includedirs({
        ".",
        blu.extdir .. "/idasdk72/include"
    })
    libdirs({
        blu.extdir .. "/idasdk72/lib/x64_win_vc_64",
        blu.extdir .. "/idasdk72/lib/x64_win_qt"
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