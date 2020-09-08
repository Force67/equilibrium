@echo off

for %%i in (code\protocol\*.fbs) do tools\build\flatc --cpp -o code\client\sync\netmessages\ %%i
for %%i in (code\protocol\*.fbs) do tools\build\flatc --csharp -o code\server\sync\netmessages\ %%i

timeout /t 3 /nobreak
