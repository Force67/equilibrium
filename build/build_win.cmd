SET mypath=%~dp0
echo %mypath:~0,-1%


@echo off
setlocal enabledelayedexpansion

bin\win\premake5 --file=../premake5.lua vs2022

for /f "usebackq tokens=*" %%i in (`%cd%/bin/win/vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBUILDDIR=%%i
)

"%MSBUILDDIR%" "%cd%/../out/Equilibrium.sln" /p:Configuration=Release -m