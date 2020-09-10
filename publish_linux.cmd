dotnet publish build/net/Server.csproj -c Release --self-contained false -r ubuntu.16.04-x64
timeout /t 10 /nobreak
