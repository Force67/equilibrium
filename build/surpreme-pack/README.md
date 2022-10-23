## Surpreme Package Manager for Premake5

This module attempts to implement a package manager for premake5 that is similar to the one used by [xmake]().

Install this module somewhere premake can find it, for example:

```
git clone https://github.com/Force67/surpreme-package-manager
```

Then put this at the top of your system script(eg: ~/.premake/premake-system.lua):

```lua
require "surpreme-package-manager"
```

> ⚠️ Note that while possible, it is not recommended to put the `require` line in
project-specific premake configuration because the "surpreme-package-manager" module will need to be installed everywhere your project is built.

After the above steps, you can start consuming packages in your premake scripts:

```lua
dependencies("imgui@1.0.0")
```

This will download the imgui package from the default repository and add it to your project. You can also specify a custom repository:

Find the package you want to use in the [surpreme package index]() and copy the package name and version. Then add the package to your project:
