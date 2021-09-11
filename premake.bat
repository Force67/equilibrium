@echo off

build\premake5 --file=premake5.lua vs2022
timeout /t 3 /nobreak
