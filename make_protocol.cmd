@echo off

rmdir /s /q "code\client\net\protocol"
rmdir /s /q "code\server\net\protocol"

for %%i in (code\protocol\*.fbs) do tools\build\flatc --cpp -o code\client\net\protocol\ %%i
for %%i in (code\protocol\*.fbs) do tools\build\flatc --csharp -o code\server\net\ %%i

timeout /t 3 /nobreak
