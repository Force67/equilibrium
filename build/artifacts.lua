-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

blu.rootdir = os.getcwd() .. '/../'
-- current dir
blu.builddir = os.getcwd()

local outdir = os.getcwd() .. '/../out/' .. _ACTION

-- %{prj.bludomain}
-- %{prj.group}
blu.bindir = outdir .. "/bin/%{cfg.platform}/%{cfg.buildcfg}/%{prj.family}"
blu.libdir = outdir .. "/lib/%{cfg.platform}/%{cfg.buildcfg}/%{prj.family}"
blu.objdir = outdir .. "/link/%{cfg.platform}/%{cfg.buildcfg}"
blu.symdir = outdir .. "/sym/%{cfg.platform}/%{cfg.buildcfg}"
blu.tempdir = outdir .. "/temp/"

-- newprojectformat tries to outsmarten me
blu.netdir = outdir .. "/clr/%{cfg.platform}"
blu.netout = outdir .. "/net"

os.mkdir(blu.tempdir)

location(outdir)

filter("language:C or C++")
  objdir(blu.objdir)
  architecture("x86_64")
filter{}

-- workaround a premake bug
if _ACTION ~= "vscode" then
  os.mkdir(outdir .. "/sym")
end

blu.extdir = path.getabsolute(os.getcwd() .."/../external")

-- This setting prevents all symbols from being exported by default
-- This has to be applied to pretty much every project, since otherwise, the dependencies of a .so
-- will still leak the symbols.
visibility("Hidden")

-- dont bloat output folder with libs
filter("kind:not StaticLib")
  targetdir(blu.bindir)
  libdirs({blu.bindir})

filter("kind:StaticLib")
  targetdir(blu.libdir)
  libdirs({blu.libdir})
-- LEAVE FILTER BARRIER
filter{}

function blu.include_root()
  includedirs(blu.rootdir)
end