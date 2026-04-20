-- Copyright (C) 2021 Vincent Hengel <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
local extroot = os.getcwd()

project("tracysdk")
  kind("StaticLib")
  language("C++")
  pubdefines("TRACY_ENABLE")
  filter("system:windows")
    pubdefines("TRACY_HAS_CALLSTACK")
  filter{}
  pubincludedirs("./tracy/public")
  files("tracy/TracyClient.cpp")
  -- v 8.2.1.+
  --pubincludedirs("./tracy/public")
  --files({
    --"tracy/public/TracyClient.cpp",
  --})

project("mbedtls")
  kind("StaticLib")
  language("C")
  includedirs({
    "mbedtls/include",
    "mbedtls/library"
  })
  pubincludedirs({
    "mbedtls/include",
  })
  pubdefines({
    "MBEDTLS_ALLOW_PRIVATE_ACCESS"
  })
  files({
    "mbedtls/library/*.c",
    "mbedtls/library/*.h",
  })

project("googlemock")
  language("C++")
  kind("StaticLib")
  pubincludedirs({
    "googletest/googlemock/include",
    "googletest/googletest/include",
    "googletest/googlemock",
    "googletest/googletest"
  })
  files({
    "googletest/googlemock/src/gmock-all.cc",
    "googletest/googletest/src/gtest-all.cc",
    --"googletest/googlemock/src/*.cc",
    --"googletest/googletest/src/*.cc",
    "googletest/googlemock/include/gmock/**.h",
    --"googletest/googletest/include/gtes/**.h",
  })

project("stackwalker")
  language("C++")
  kind("StaticLib")
  filter("system:not windows")
    kind("None")
  filter{}
  pubincludedirs({
    "stackwalker/Main/StackWalker",
  })
  files({
    "stackwalker/Main/StackWalker/StackWalker.cpp",
    "stackwalker/Main/StackWalker/StackWalker.h",
  })

project("gtest-memleak-detector")
  language("C++")
  kind("StaticLib")
  filter("system:not windows")
    kind("None")
  filter{}
  pubincludedirs({
    "googletest/googletest/include",
    "googletest/googletest",
    "gtest-memleak-detector/include",
    "stackwalker/Main",
  })
  files({
    "gtest-memleak-detector/include/gtest_memleak_detector/gtest_memleak_detector.h",
    "gtest-memleak-detector/src/*.cpp",
  })

project("lz4")
  language("C")
  kind("StaticLib")
  pubincludedirs({
    "./lz4/lib",
  })
  files({
    "lz4/lib/*.c",
    "lz4/lib/*.h"
  })

project("mimalloc")
  language("C")
  kind("StaticLib")
  pubincludedirs({
    "./mimalloc/include",
  })
  files({
    "mimalloc/src/stats.c",
    "mimalloc/src/random.c",
    "mimalloc/src/os.c",
    "mimalloc/src/bitmap.c",
    "mimalloc/src/arena.c",
    "mimalloc/src/segment-cache.c",
    "mimalloc/src/segment.c",
    "mimalloc/src/page.c",
    "mimalloc/src/alloc.c",
    "mimalloc/src/alloc-aligned.c",
    "mimalloc/src/alloc-posix.c",
    "mimalloc/src/heap.c",
    "mimalloc/src/options.c",
    "mimalloc/src/init.c",
  })

project("musl")
  language("C")
  cdialect("C11")
  kind("StaticLib")
  pubincludedirs({
    "musl/arch/generic",
    "musl/src/internal",
    "musl/src/include", -- because fuck you that's why
    "musl/include",
  })
  filter("architecture:x86")
    pubincludedirs({
      "musl/arch/x86",
      "_override/musl/arch/x86",
      "_override/musl/src/internal"
    })
  filter("architecture:x86_64")
    pubincludedirs({
      "musl/arch/x86_64",
      "_override/musl/arch/x86_64",
      "_override/musl/src/internal"
    })
  filter{}
  --includedirs("musl/src/internal")
  files({
    "musl/src/**.c",
    "musl/src/**.h",
    "musl/include/**.h"
  })
  filter("architecture:x86")
    files({
      "musl/arch/x86/**.c",
      "musl/arch/x86/**.h",
    })
  filter("architecture:x86_64")
    files({
      "musl/arch/x86_64/**.c",
      "musl/arch/x86_64/**.h",
    })
  filter{}
  -- TODO: investigate why hidden is not defined properly.
  --       it's defined in the musl headers, but not in the musl source
  -- TODO: include assembly files (.S)
  -- of course, don't pull in the default CRT
  buildoptions({"-nostdinc", "-ffreestanding"})
  defines("hidden")

function include_musl()
  linkoptions({"-nostdlib", "-nostdinc", "-fno-builtin", "-fno-stack-protector"})
end

function include_wayland()
  pubincludedirs({
    extroot .. "/wayland/include",
    extroot .. "/wayland/wayland-generated"
  })
end
