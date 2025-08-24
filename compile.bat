@echo off
setlocal enabledelayedexpansion
chcp 936>nul

echo 开始编译 Cloud Studio Chat，请等候。
echo.
echo 请确保你已经安装了Mingw编译器并且添加到了Path。

rem (c) 2025 Piaoztsdy

echo.
echo 现在开始删除编译前的文件...
del client.exe server.exe >nul 2>&1

echo.
echo 现在开始编译二进制文件...

rem 编译 client.exe
g++ client.cpp -o client.exe -mwindows -lws2_32 -static -pthread -static-libgcc -static-libstdc++
set CLIENT_COMP_STATUS=%errorlevel%
if !CLIENT_COMP_STATUS! equ 0 (
    echo client.exe 编译成功
) else (
    echo client.exe 编译失败
)

echo.

rem 编译 server.exe
g++ server.cpp -o server.exe -mconsole -lws2_32 -static -pthread -static-libgcc -static-libstdc++
set SERVER_COMP_STATUS=%errorlevel%
if !SERVER_COMP_STATUS! equ 0 (
    echo server.exe 编译成功
) else (
    echo server.exe 编译失败
)

echo.

rem 统一检查编译结果
if !CLIENT_COMP_STATUS! neq 0 (
    set comperr=1
)
if !SERVER_COMP_STATUS! neq 0 (
    set comperr=1
)

if defined comperr (
    echo.
    echo.
    echo 部分代码编译过程中出现错误，请查看g++的报错，并且提交issue。
    echo.
    echo 提交格式：
    echo 1. 你的环境
    echo 1.1 你的系统名称（如 Windows 10）
    echo 1.2 你的编译器版本（在命令行输入 g++ --version）
    echo 1.3 你的系统架构（只能为 x64，如果是 x86 或者 arm64 则不受理）
    echo 2. 你的 g++ 报错内容
    pause
    exit /b 1
) else (
    echo 全部编译成功。
    pause
)

endlocal