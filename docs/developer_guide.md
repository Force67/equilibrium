# About the build system
Equilibrium framework is using a customized build system built ontop of premake5. Check out `build/` for more info

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
    -- header only:
```