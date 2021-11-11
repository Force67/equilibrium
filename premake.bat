@echo off
build\bin\premake5 --file=premake5.lua vs2022
:: Determine if we were started by double click
timeout /t 3 /nobreak