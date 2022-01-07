# Using 4Coder

## Prerequisites
It is strongely recommended to install the [4coder fleury layer](https://github.com/ryanfleury/4coder_fleury) in order to get proper syntax highlighting support.

In addition you must tweak your 4coder config, which can be found in the main `<4coderdir>`.
Configure your indent to be two spaces, as per the cxx style guide.
```
...
virtual_whitespace_regular_indent = 2;
...
indent_with_tabs = false;
indent_width = 2;
...
```
## Opening the project:
Open 4Coder from the `equilibrium/` directory. Press `Alt+X` to open the command and execute the `load_project/` command.

Now that the project is opened you can switch files by pressing `CTRL+I`.

TODO: Toggle configuration script...
## Keybindings
* `F1` - Build in release
* `F2` - Build in debug
* `F4` - Execute premake