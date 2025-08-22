@echo off
chcp 65001>nul

echo 开始编译 Cloud Studio Chat，请等候。
echo.
echo 请确保你已经安装了Mingw编译器并且添加到了Path。

rem (c) 2025 Piaoztsdy

g++ client.cpp -o client.exe -mwindows -lws2_32
g++ server.cpp -o server.exe -mconsole -lws2_32