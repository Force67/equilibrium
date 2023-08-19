#ifndef VERSION_IT_INCLUDE_GUARD
#define VERSION_IT_INCLUDE_GUARD
#define VERSION_IT_VERSION "0.8.47"
#define VERSION_IT_USING_SEMANTIC_VERSIONING
#define VERSION_IT_MAJOR 0
#define VERSION_IT_MINOR 8
#define VERSION_IT_PATCH 47

#ifdef __cplusplus
namespace version_it
{
  constexpr const char kVersionString[] = "0.8.47";
  constexpr int major = 0;
  constexpr int minor = 8;
  constexpr int patch = 47;
}
#endif

#ifdef __cplusplus
namespace version_it
{
  static inline const char* kChangelogEntries[53] =
  {
    "test regressions",
    "submodule updates",
    "add custom build asset support",
    "small qol stuff",
    "i believe it works but i dont trust it",
    "bucketallocator refactorings",
    "make virtual address mapping work..",
    "better documentation",
    "new vector method",
    "fmtlib v10",
    "stringref->isempty",
    "roll flatc",
    "hash support",
    "Almost complete vector impl",
    "bump fmt",
    "linux regressions",
    "eek",
    "Only build Debug and Shipping",
    "remove SKIA",
    "Fix invalid path resolving",
    "peek for codepoint iterator.",
    "add utf8 iterator.",
    "fix base_shared target",
    "fix invalid character in powershell",
    "Added CI (Test #1)",
    "add missing move ctor",
    "Commandline::hasSwitch",
    "small code cleanups",
    "Not calling dtors in all cases.",
    "fix hash functions",
    "Add djb2, elf, murmur",
    "broken command line class",
    "Always crash on invalid access.",
    "preliminary own std::function replacement",
    "fix eq_database on linux",
    "correctness fixes.",
    "Some linux fixes",
    "gemerate intellisense info",
    "rewrite entire vscode impl",
    "yeet old node stuff",
    "some linux fixes",
    "bump premake on linux to beta2.",
    "super fast string search",
    "assign proper test",
    "restore checks on linux.",
    "we have liftoff!",
    "ignore files when instructed.",
    "platform specific files",
    "small fixes",
    "compile_commands proper support for clangd",
    "linux buildsystem perms",
    "even more portability",
    "some portability changes"
  };
}
#endif

// 0.8.47 fix 6904c2405584bcd682651dd24d4318ff84a1541e 2023-08-19 Vincent Hengel test regressions
// 0.8.47 tweak eee870b34ea97e47e4f92e929008847a83b00a03 2023-08-15 Vincent Hengel submodule updates
// 0.8.47 tweak a4738751b3f8ed501140cef55ca456088f4e9903 2023-08-15 Vincent Hengel add custom build asset support
// 0.8.47 tweak 19d6fbc9dbb0bb91d026b4425f36a5b06c84c39b 2023-07-30 Vincent Hengel small qol stuff
// 0.8.47 tweak fbe58ad5faf7438dae54589daa0b003efddf2226 2023-07-23 Vincent Hengel i believe it works but i dont trust it
// 0.8.47 tweak ea7b921b9d49700df41ee0b753f72f2abec2f8ff 2023-07-23 Vincent Hengel bucketallocator refactorings
// 0.8.47 tweak d3c3ae7172cb9c8ad3b7ee22ea5ae0f6af8d9299 2023-07-23 Vincent Hengel make virtual address mapping work..
// 0.8.47 tweak(mem) f0546b512c368cea28acfa6999c1ff81035398ea 2023-07-22 Vincent Hengel better documentation
// 0.8.47 tweak 00ab7ddaba84dd4db13d4b8bd6d2f58b9c255a8a 2023-07-21 Vincent Hengel new vector method
// 0.8.47 tweak 096c13582e31dfdb8bd21092a5e355ce86a3e3d4 2023-07-20 Vincent Hengel fmtlib v10
// 0.8.47 tweak 784aa95aa1e5cb7643ffa47a4be0b6e86fe016f9 2023-05-14 Vincent Hengel stringref->isempty
// 0.8.47 tweak 961bfb8b4020fc7886c8cf43a8cf7d5132667881 2023-04-22 Vincent Hengel roll flatc
// 0.8.47 tweak(base/stringref) 1453633f3aa282fbe6f9e41a561bfe54add0e57d 2023-04-15 Vincent Hengel hash support
// 0.8.47 feat(base/containers) 2a7a60a3a8f0fc50a2ac420c2b45b4d059776f0d 2023-04-14 Vincent Hengel Almost complete vector impl
// 0.8.47 tweak 3140daf26df48cddb2eb415be94098148c54fb95 2023-04-14 Vincent Hengel bump fmt
// 0.8.47 fix b057bc6d92d12cfccd9ad8f9c301a4ed35c033c7 2023-03-09 Vincent Hengel linux regressions
// 0.8.47 feat(rust) b89120594a9e485fcb6322a8fa102a9b34a523ea 2023-03-08 Vincent Hengel eek
// 0.8.47 tweak(CI) cac03c6ad2b6f015fe8ad3acf1dfa2fbb51887b6 2023-01-19 BlackOfWorld Only build Debug and Shipping
// 0.8.47 tweak 99994b1fc517ccbef81355a7925f5b783330c4e0 2023-01-19 Vincent Hengel remove SKIA
// 0.8.47 fix(CI) c3ea0b052f5ac8ce54dfb01bc0519c948a0c0425 2023-01-15 BlackOfWorld Fix invalid path resolving
// 0.8.47 tweak 9803797959147a9df7d4b1ebdcfae793f5ace480 2023-01-15 Vincent Hengel peek for codepoint iterator.
// 0.8.47 tweak 0a667e08dd8dcfb7d60be5b926b5271d5e70af9a 2023-01-14 Vincent Hengel add utf8 iterator.
// 0.8.47 tweak b61b32e1af8de82e73cf5eef0adeef25b9f9fc55 2023-01-12 Vincent Hengel fix base_shared target
// 0.8.47 fix(CI) 8b88582f266c6d3b9d36e755105e066c1b974bd5 2023-01-11 BlackOfWorld fix invalid character in powershell
// 0.8.47 feat(CI) eaf2c625f191440f874f0cc956082763555d58c9 2023-01-11 BlackOfWorld Added CI (Test #1)
// 0.8.47 fix(base/vector) f0e17fcb2c27f0ea47a9a2d98d87367075ee1f50 2022-12-31 Vincent Hengel add missing move ctor
// 0.8.47 tweak c4b6102285f4ada98ed476a31da4484616e1c367 2022-12-28 Vincent Hengel Commandline::hasSwitch
// 0.8.47 tweak 8e3e661c028ab7ac3e9722e6e48951f74a264d43 2022-12-28 Vincent Hengel small code cleanups
// 0.8.47 hotfix(BASE/vector) c1a16e6470dbbdbc24e5ed2468316aae9be56e61 2022-12-28 Vincent Hengel Not calling dtors in all cases.
// 0.8.47 tweak be81e0773a7b90fb383221910e0533d40209ebc9 2022-12-27 Vincent Hengel fix hash functions
// 0.8.47 tweak(Hash) 10669c27f7d58efc3c8c36911eed3de2665b8021 2022-12-27 BlackOfWorld Add djb2, elf, murmur
// 0.8.47 tweak 86def80084274c5012afeeae0f5dc0960dddc3ca 2022-12-26 Vincent Hengel broken command line class
// 0.8.47 fix(string_ref) 410873e8f54c44dc97b238346313f0231a099e09 2022-12-26 Vincent Hengel Always crash on invalid access.
// 0.8.47 tweak 76a02d7cd18f4d97ca7915eed3a5a1d9bdee9967 2022-12-26 Vincent Hengel preliminary own std::function replacement
// 0.8.47 tweak b53f46d4313ef93a9239ab9edde9935ab8bf8155 2022-12-21 Vincent Hengel fix eq_database on linux
// 0.8.47 tweak(base) b204eb75c6804fdff9363b6a8d9b9eb9a61ecc3f 2022-12-17 Vincent Hengel correctness fixes.
// 0.8.47 tweak f1fb491d15dfd0bfabd56265432a25d4a3b6604f 2022-09-23 Vincent Hengel Some linux fixes
// 0.8.47 tweak(vscode) 2432e292c03c39235f5124fcff661c1bb25cdc4d 2022-09-11 Vincent Hengel gemerate intellisense info
// 0.8.47 feat(build) 51b0907bbf9f03b097b12a5064161bd5a2b9f199 2022-09-11 Vincent Hengel rewrite entire vscode impl
// 0.8.47 tweak d6bb00caa34fe0e36da018dd76c20dacc80fd4a1 2022-09-10 Vincent Hengel yeet old node stuff
// 0.8.47 tweak 524596b8a904cb8424bec639e64269074f5382a6 2022-09-04 Vincent Hengel some linux fixes
// 0.8.47 tweak(build) 329fcf7fe01b6b49b79ae06674c9a20ddee42afb 2022-09-04 Vincent Hengel bump premake on linux to beta2.
// 0.8.47 feat(strings) 72c8de77dcf71c3124e438bac5a097491bb6c813 2022-06-18 Vincent Hengel super fast string search
// 0.8.47 fix 171e76af20bb5937147b16f05ed22cff60d00f4a 2022-06-18 Vincent Hengel assign proper test
// 0.8.47 tweak 0b2fbc9d60289796977e424a32eca43c301efc8f 2022-06-18 Vincent Hengel restore checks on linux.
// 0.8.47 feat(linux) 58b4e8e9deb62b46177b501468c156a1c7a5466b 2022-06-18 Vincent Hengel we have liftoff!
// 0.8.47 feat(compile_commands) dcb04a3c4b129abafa89daabf029b1a74fbd56b1 2022-06-15 Vincent Hengel ignore files when instructed.
// 0.8.47 feat(buildsys) aa2aa8e47e9ec67800d36d2ef94897567faccd87 2022-06-15 Vincent Hengel platform specific files
// 0.8.47 tweak b1bdf6d1b0c0c143f18c8b27606a0e779534fa67 2022-06-12 Vincent Hengel small fixes
// 0.8.47 tweak 77a012646df0dc8a6acbcc3bd8a2c154d8f7cc32 2022-06-12 Vincent Hengel compile_commands proper support for clangd
// 0.8.47 fix fb0751994aa13a788970b3289c557ee8e11d94ec 2022-06-12 Vincent Hengel linux buildsystem perms
// 0.8.47 tweak f55754b663c8f8cdcd638e891ccb623582420930 2022-06-10 Vincent Hengel even more portability
// 0.8.47 tweak 123e253f6ff725a67f18bd62303a1ca4d2561cb8 2022-06-10 Vincent Hengel some portability changes
#endif