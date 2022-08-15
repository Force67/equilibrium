# Project Architecture

Everything in the equilibrium source tree is using base as a sort of standard library. Then there are global components which are used by all projects (in `equilibrium/components`), and then there are project specific components which are used by individual projects. This is done to ensure that programs remain modular and can be tested independently of each other.

For more information on how to use/consume the base library, see its [readme](../base/readme.md).

# About the build system
Equilibrium framework is using a customized build system built on top of premake5. Check out `build/` for more info

## Dependency management
Dependencies generally go into the toplevel `external/` directory. They can be configured using the following pub_ functions, which mark given data as transitive:
```
pubincludedirs(<string list of dirs>...)
pubdefines(<string list of defines>...)
```
## Adding new components
In order to add new components, you have to declare them as follows:
```
component("<componentname>")
    files({
        "*.cc",
        "*.h",
    })
    dependencies("fmtlib") -- list of dependencies to link against and resolve transitive data from
```