: # This is a special script which intermixes both sh
: # and cmd code. It is written this way because it is
: # used in system() shell-outs directly in otherwise
: # portable code. See https://stackoverflow.com/questions/17510688
: # for details.
: # Please do not save this file with a windows editor, as it requires
: # linux style line endings.
:<<"::CMDLITERAL"
cd build
premake_win.bat
::CMDLITERAL
:;
bash build / premake_linux.sh:;
exit