#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <map>

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