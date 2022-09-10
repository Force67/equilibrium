-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

blu.rootdir = os.getcwd() .. '/../'
blu.builddir = os.getcwd()

local outdir = os.getcwd() .. '/../out/' .. _ACTION

blu.bindir = outdir .. "/bin/%{cfg.platform}/%{cfg.buildcfg}"
blu.libdir = outdir .. "/lib/%{cfg.platform}/%{cfg.buildcfg}"
blu.objdir = outdir .. "/link/%{cfg.platform}/%{cfg.buildcfg}"
blu.symdir = outdir .. "/sym/%{cfg.platform}/%{cfg.buildcfg}"

-- newprojectformat tries to outsmarten me
blu.netdir = outdir .. "/clr/%{cfg.platform}"
blu.netout = outdir .. "/net"

location(outdir)

filter("language:C or C++")
  objdir(blu.objdir)
  libdirs({blu.libdir})
  architecture("x86_64")
filter{}

-- workaround a premake bug
if _ACTION ~= "vscode" then
  os.mkdir(outdir .. "/sym")
end

blu.extdir = path.getabsolute(os.getcwd() .."/../external")

-- dont bloat output folder with libs
filter("kind:not StaticLib")
    targetdir(blu.bindir)
filter("kind:StaticLib")
    targetdir(blu.libdir)

-- LEAVE FILTER BARRIER
filter{}