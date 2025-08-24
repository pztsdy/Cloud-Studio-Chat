#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <map>

// 全局变量
const std::wstring VERSION = L"1.2";
const std::wstring client_name = L"Cloud Studio Chat";

// server.cpp
/// 函数声明

void broadcastMessage(const std::string &message);
void sendAdminMessage(const std::string &message);
void kickUser(const std::string &identifier);
void runServerCommands();
bool initializeNetwork();
void cleanupNetwork();

// client.cpp
/// 控件 ID 声明
#define ID_EDIT_RECV 101
#define ID_EDIT_SEND 102
#define ID_BUTTON_SEND 103
#define ID_EDIT_USERNAME 104
#define ID_EDIT_SERVERIP 105
#define ID_BUTTON_OK 106
#define ID_BUTTON_CANCEL 107
#define ID_MENU_ABOUT 108
#define ID_STATIC_PORT 109
#define ID_EDIT_PORT 110
#define ID_MENU_PIN 111
#define ID_MENU_QUIT 112
#define ID_MENU_CANCEL_PIN 113
#define ID_UPDATE_LOG 114

/// 更新日志

std::wstring logContent = std::wstring(L"更新日志:\r\n")
    + L"\r\n"
    + L"当前版本: " + VERSION + L"\r\n\r\n"
    + L"版本1.2更新内容:\r\n"
    + L"- 修改功能易于开发\r\n"
    + L"- 进行了部分代码重构\r\n"
    + L"- 修复了一些已知问题，提升兼容性，现理论已经支持 32 位系统\r\n\r\n"
    + L"版本1.1更新内容：\r\n"
    + L"- 移动置顶功能的位置\r\n"
    + L"- 添加“更新日志”功能\r\n"
    + L"- 修复了一些已知问题，提升兼容性，现理论已经支持 Windows XP 系统"
;