-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

-- define how to handle fbs files.
rule("CompileFlatbuffers")
  display("Flatc")
  fileextension(".fbs")
  buildmessage('Compiling %(Filename) with Flatc')
  filter("system:windows")
    buildcommands(blu.rootdir .. '/build/bin/win/flatc --cpp -o %(RelativeDir) %(FullPath)')
  filter("system:linux")
    buildcommands(blu.rootdir .. '/build/bin/linux/flatc --cpp -o %(RelativeDir) %(FullPath)')
  filter{}
  buildoutputs('%(RelativeDir)%(Filename)_generated.h')