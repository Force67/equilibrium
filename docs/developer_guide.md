# Project Architecture

It is important to understand that equilibrium functions as a mono-repository that provides a set of common code blocks and build tools for all projects built by VH-Tech. First there are global components which are used by all projects (in `equilibrium/components`), and then there are project specific components which are used by individual projects. 
The projects live in the `/projects/` dir and are not tracked from equilibrium, they merely depend on equilibrium but are cloned into the projects folder.

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