@echo off
setlocal enabledelayedexpansion
chcp 936>nul

echo ��ʼ���� Cloud Studio Chat����Ⱥ�
echo.
echo ��ȷ�����Ѿ���װ��Mingw������������ӵ���Path��

rem (c) 2025 Piaoztsdy

echo.
echo ���ڿ�ʼɾ������ǰ���ļ�...
del client.exe server.exe >nul 2>&1

echo.
echo ���ڿ�ʼ����������ļ�...

rem ���� client.exe
g++ client.cpp -o client.exe -mwindows -lws2_32 -static -pthread -static-libgcc -static-libstdc++
set CLIENT_COMP_STATUS=%errorlevel%
if !CLIENT_COMP_STATUS! equ 0 (
    echo client.exe ����ɹ�
) else (
    echo client.exe ����ʧ��
)

echo.

rem ���� server.exe
g++ server.cpp -o server.exe -mconsole -lws2_32 -static -pthread -static-libgcc -static-libstdc++
set SERVER_COMP_STATUS=%errorlevel%
if !SERVER_COMP_STATUS! equ 0 (
    echo server.exe ����ɹ�
) else (
    echo server.exe ����ʧ��
)

echo.

rem ͳһ��������
if !CLIENT_COMP_STATUS! neq 0 (
    set comperr=1
)
if !SERVER_COMP_STATUS! neq 0 (
    set comperr=1
)

if defined comperr (
    echo.
    echo.
    echo ���ִ����������г��ִ�����鿴g++�ı��������ύissue��
    echo.
    echo �ύ��ʽ��
    echo 1. ��Ļ���
    echo 1.1 ���ϵͳ���ƣ��� Windows 10��
    echo 1.2 ��ı������汾�������������� g++ --version��
    echo 1.3 ���ϵͳ�ܹ���ֻ��Ϊ x64������� x86 ���� arm64 ������
    echo 2. ��� g++ ��������
    pause
    exit /b 1
) else (
    echo ȫ������ɹ���
    pause
)

endlocal