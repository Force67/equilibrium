-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

-- define how to handle fbs files.
rule("CompileFlatbuffers")
  display("Flatc")
  fileextension(".fbs")
  buildmessage('Compiling %(Filename) with Flatc')
  buildcommands(blu.rootdir .. '/build/bin/win/flatc --cpp -o %(RelativeDir) %(FullPath)')
  buildoutputs('%(RelativeDir)%(Filename)_generated.h')