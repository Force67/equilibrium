@echo off

for %%i in (code\protocol\*.fbs) do tools\build\flatc --cpp -o code\client\net\protocol\ %%i
for %%i in (code\protocol\*.fbs) do tools\build\flatc --csharp -o code\server\net\ %%i

timeout /t 3 /nobreak
