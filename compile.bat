@echo off
chcp 65001>nul

echo 开始编译 Cloud Studio Chat，请等候。
echo.
echo 请确保你已经安装了Mingw编译器并且添加到了Path。

rem (c) 2025 Piaoztsdy

echo.
echo 现在开始删除编译前的文件...
echo 请输入y以确认删除。
set /p confirm=确认删除吗？(y/n)
if "%confirm%"=="y" (
    del client.exe -f
    del server.exe -f
)

echo.
echo 现在开始编译二进制文件...
g++ client.cpp -o client.exe -mwindows -lws2_32
g++ server.cpp -o server.exe -mconsole -lws2_32

if exist client.exe (
    echo client.exe 编译成功
) else (
    set er=2
    echo client.exe 编译失败
)

echo.

if exist server.exe (
    echo server.exe 编译成功
) else (
    set er=2
    echo server.exe 编译失败
)

if %er% == 2 (
    echo 编译过程中出现错误，请查看g++的报错，并且提交issue。
    echo.
    echo 提交格式：
    echo  1. 你的环境
    echo   1.1 你的系统名称（如 Windows 10）
    echo   1.2 你的编译器版本（在命令行输入 g++ --version）
    echo   1.3 你的系统架构（只能为 x64，如果是 x86 或者 arm64 则不受理）
    echo  2. 你的 g++ 报错内容
    exit /b 1
)