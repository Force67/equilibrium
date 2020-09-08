-- Copyright (c) Stronkat. All rights reserved.

local rootdir = os.getcwd() .. '/../../build'
blu.bindir = rootdir .. "/bin/%{cfg.platform}/%{cfg.buildcfg}"
blu.libdir = rootdir .. "/lib/%{cfg.platform}/%{cfg.buildcfg}"
blu.objdir = rootdir .. "/link/%{cfg.platform}/%{cfg.buildcfg}"
blu.symdir = rootdir .. "/sym/%{cfg.platform}/%{cfg.buildcfg}"

-- newprojectformat tries to outsmarten me
blu.netdir = rootdir .. "/clr/%{cfg.platform}"
blu.netout = rootdir .. "/net"

location(rootdir)

filter("language:C or C++")
    location(rootdir)
    objdir(blu.objdir)
    libdirs({blu.libdir})
    architecture("x86_64")
filter{}

-- workaround a premake bug
os.mkdir(rootdir .. "/sym")
blu.extdir = path.getabsolute(os.getcwd() .."/../../external")

-- dont bloat output folder with libs
filter("kind:not StaticLib")
    targetdir(blu.bindir)
filter("kind:StaticLib")
    targetdir(blu.libdir)