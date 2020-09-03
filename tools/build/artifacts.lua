-- Copyright (c) Stronkat. All rights reserved.

local rootdir = os.getcwd() .. '/../../build'
blu.bindir = rootdir .. "/bin/%{cfg.platform}/%{cfg.buildcfg}"
blu.libdir = rootdir .. "/lib/%{cfg.platform}/%{cfg.buildcfg}"
blu.objdir = rootdir .. "/obj/%{cfg.platform}/%{cfg.buildcfg}"
blu.symdir = rootdir .. "/sym/%{cfg.platform}/%{cfg.buildcfg}"

location(rootdir)
objdir(blu.objdir)
libdirs({blu.libdir})

-- workaround a premake bug
os.mkdir(rootdir .. "/sym")
blu.extdir = path.getabsolute(os.getcwd() .."/../../external")

-- dont bloat output folder with libs
filter("kind:not StaticLib")
    targetdir(blu.bindir)
filter("kind:StaticLib")
    targetdir(blu.libdir)