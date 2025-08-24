#define UNICODE
#define _UNICODE

#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include "csc_basic.h"

#pragma comment(lib, "ws2_32.lib")

// 用户端错误：exit(1)
// 系统类错误：exit(2)
// 网络错误：exit(3)
// 其他错误：exit(4)

bool isPinned = false;
HINSTANCE hInst;
HWND hEditRecv;
HWND hEditSend;
SOCKET clientSocket;
std::string serverIP;
std::string username; // 用户名
int serverPort = 6543;
HFONT hFont; // 字体句柄

// 创建字体
HFONT CreateCustomFont()
{
    return CreateFont(
        -14,                         // 高度
        0,                           // 宽度
        0,                           // 倾斜角度
        0,                           // 旋转角度
        FW_NORMAL,                   // 细体
        FALSE,                       // 斜体
        FALSE,                       // 下划线
        FALSE,                       // 删除线
        DEFAULT_CHARSET,             // 字符集
        OUT_DEFAULT_PRECIS,          // 输出精度
        CLIP_DEFAULT_PRECIS,         // 剪裁精度
        CLEARTYPE_NATURAL_QUALITY,   // 抗锯齿
        DEFAULT_PITCH | FF_DONTCARE, // 字符集和间距
        L"Microsoft YaHei"            // 字体名称
    );
}

void receiveMessages()
{
    char buffer[65536];
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, 65536, 0)) > 0)
    {
        buffer[bytesReceived] = '\0';
        std::string message(buffer);
        message += "\r\n---------------------------------\r\n";

        // 保存当前选择区
        DWORD startSel = 0, endSel = 0;
        SendMessage(hEditRecv, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel);

        // 将光标移到末尾插入消息
        int textLen = GetWindowTextLength(hEditRecv);
        SendMessage(hEditRecv, EM_SETSEL, textLen, textLen);
        SendMessageA(hEditRecv, EM_REPLACESEL, FALSE, (LPARAM)message.c_str());
        // 恢复原选择区
        SendMessage(hEditRecv, EM_SETSEL, startSel, endSel);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // 创建菜单
        HMENU hMenu = CreateMenu();
        HMENU hHelpMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, ID_MENU_PIN, L"置顶");
        AppendMenu(hMenu, MF_STRING, ID_MENU_CANCEL_PIN, L"取消置顶");
        
        AppendMenu(hHelpMenu, MF_STRING, ID_UPDATE_LOG, L"更新日志");
        AppendMenu(hHelpMenu, MF_STRING, ID_MENU_ABOUT, L"关于");
        AppendMenu(hHelpMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hHelpMenu, MF_STRING, ID_MENU_QUIT, L"退出");
        
        // 将 hHelpMenu 作为弹出菜单添加到主菜单 hMenu
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"帮助");
        SetMenu(hwnd, hMenu);

        hFont = CreateCustomFont();

        hEditRecv = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                   WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                   10, 10, 460, 300, hwnd, (HMENU)ID_EDIT_RECV, hInst, NULL);

        hEditSend = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                   WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL,
                                   10, 320, 360, 40, hwnd, (HMENU)ID_EDIT_SEND, hInst, NULL);

        HWND hButtonSend = CreateWindow(L"BUTTON", L"发送",
                                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                        380, 320, 90, 40, hwnd, (HMENU)ID_BUTTON_SEND, hInst, NULL);

        // 设置控件字体
        SendMessage(hEditRecv, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditSend, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hButtonSend, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_SEND)
        {
            char buffer[65536];
            GetWindowTextA(hEditSend, buffer, 65536);
            if (strlen(buffer) > 0)
            {
                std::string msgToSend = "[" + username + "]: " + buffer;
                send(clientSocket, msgToSend.c_str(), msgToSend.length(), 0);
                SetWindowText(hEditSend, L"");
            }
        }
        else if (LOWORD(wParam) == ID_MENU_PIN)
        {
            isPinned = true;
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            HMENU hMenu = GetMenu(hwnd);
            HMENU hSubMenu = GetSubMenu(hMenu, 0);
            CheckMenuItem(hSubMenu, ID_MENU_PIN, MF_CHECKED);
            CheckMenuItem(hSubMenu, ID_MENU_CANCEL_PIN, MF_UNCHECKED);
        }
        else if (LOWORD(wParam) == ID_MENU_CANCEL_PIN)
        {
            isPinned = false;
            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            HMENU hMenu = GetMenu(hwnd);
            HMENU hSubMenu = GetSubMenu(hMenu, 0);
            CheckMenuItem(hSubMenu, ID_MENU_PIN, MF_UNCHECKED);
            CheckMenuItem(hSubMenu, ID_MENU_CANCEL_PIN, MF_UNCHECKED);
        }
        else if (LOWORD(wParam) == ID_MENU_ABOUT)
        {
            std::wstring aboutMsg = client_name + L"\n作者: Piaoztsdy (Luogu UID: 1252756)\n版本: " + VERSION;
            MessageBoxW(hwnd, aboutMsg.c_str(), L"关于", MB_OK | MB_ICONINFORMATION);
        }
        else if (LOWORD(wParam) == ID_MENU_QUIT)
        {
            PostQuitMessage(0);
        } else if (LOWORD(wParam) == ID_UPDATE_LOG) {
            // 弹出可复制窗口显示更新日志
            HWND hLogWnd = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"更新日志",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                CW_USEDEFAULT, CW_USEDEFAULT, 450, 350, NULL, NULL, hInst, NULL);
            SendMessage(hLogWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SetWindowText(hLogWnd, logContent.c_str());
        }
        break;

    case WM_DESTROY:
        DeleteObject(hFont); // 删除字体
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK InputDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HFONT hDlgFont = NULL;

    switch (msg)
    {
    case WM_CREATE:
    {
        hDlgFont = CreateCustomFont();

        // 创建对话框控件
        HWND hLabelUser = CreateWindow(L"STATIC", L"用户名:",
                                       WS_CHILD | WS_VISIBLE,
                                       10, 10, 70, 25, hwnd, NULL, hInst, NULL);

        HWND hEditUser = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                        90, 10, 180, 30, hwnd, (HMENU)ID_EDIT_USERNAME, hInst, NULL);

        HWND hLabelIP = CreateWindow(L"STATIC", L"服务器IP:",
                                     WS_CHILD | WS_VISIBLE,
                                     10, 40, 70, 25, hwnd, NULL, hInst, NULL);

        HWND hEditIP = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                      WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                      90, 40, 180, 30, hwnd, (HMENU)ID_EDIT_SERVERIP, hInst, NULL);

        HWND hBtnOK = CreateWindow(L"BUTTON", L"确定",
                                   WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                   90, 80, 80, 25, hwnd, (HMENU)ID_BUTTON_OK, hInst, NULL);

        HWND hBtnCancel = CreateWindow(L"BUTTON", L"取消",
                                       WS_CHILD | WS_VISIBLE,
                                       180, 80, 80, 25, hwnd, (HMENU)ID_BUTTON_CANCEL, hInst, NULL);

        // 设置对话框控件字体
        SendMessage(hLabelUser, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
        SendMessage(hEditUser, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
        SendMessage(hLabelIP, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
        SendMessage(hEditIP, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
        SendMessage(hBtnOK, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
        SendMessage(hBtnCancel, WM_SETFONT, (WPARAM)hDlgFont, TRUE);

        SetFocus(hEditUser);
        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BUTTON_CANCEL:
            PostQuitMessage(0);
            break;
        case ID_BUTTON_OK:
        {
            char userBuffer[80], ipBuffer[40]; // 不需要 portBuffer
            GetDlgItemTextA(hwnd, ID_EDIT_USERNAME, userBuffer, 80);
            GetDlgItemTextA(hwnd, ID_EDIT_SERVERIP, ipBuffer, 40);

            if (strlen(userBuffer) == 0 || strlen(ipBuffer) == 0)
            {
                MessageBox(hwnd, L"请填写完整信息", L"提示", MB_OK | MB_ICONERROR);
                exit(1);
            }

            username = userBuffer;
            serverIP = ipBuffer;

            serverPort = 6543; // 默认端口
            DestroyWindow(hwnd);
            break;
        }
        }
        break;
    case WM_CLOSE:
        DeleteObject(hDlgFont); // 删除字体
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    hInst = hInstance;

    // 注册输入对话框窗口类
    WNDCLASSEX wcDlg = {0};
    wcDlg.cbSize = sizeof(WNDCLASSEX);
    wcDlg.lpfnWndProc = InputDlgProc;
    wcDlg.hInstance = hInstance;
    wcDlg.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcDlg.lpszClassName = L"InputDialog";
    RegisterClassEx(&wcDlg);

    // 创建并显示输入对话框
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        L"InputDialog",
        L"登录",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        (GetSystemMetrics(SM_CXSCREEN) - 300) / 2, // 居中显示
        (GetSystemMetrics(SM_CYSCREEN) - 200) / 2,
        300, 150,
        NULL, NULL, hInstance, NULL);

    if (!hDlg)
    {
        MessageBoxW(NULL, L"无法创建登录对话框", L"错误", MB_ICONEXCLAMATION | MB_OK);
        exit(2);
    }

    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (!IsWindow(hDlg))
            break;
    }

    // 只判断 serverIP
    if (serverIP.empty())
    {
        MessageBoxW(NULL, L"未输入服务器IP，程序退出。", L"错误", MB_ICONEXCLAMATION | MB_OK);
        exit(1);
    }

    // 注册主窗口类
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ChatWindow";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBoxW(NULL, L"注册窗口失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        exit(2);
    }
    // 创建主窗口
    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"ChatWindow",
        client_name.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 430,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBoxW(NULL, L"创建窗口失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        exit(2);
    }

    // 初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        MessageBoxW(NULL, L"WSAStartup失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        exit(3);
    }

    // 创建套接字并连接到服务器
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    // 检查 serverIP 是否为空
    if (serverIP.empty())
    {
        MessageBoxW(NULL, L"服务器IP为空，无法连接！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        MessageBoxW(NULL, L"连接失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        closesocket(clientSocket);
        WSACleanup();
        exit(3);
    }

    // 连接成功后立即发送用户名
    send(clientSocket, username.c_str(), username.length(), 0);

    std::thread(receiveMessages).detach();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(clientSocket);
    WSACleanup();
    return (int)msg.wParam;
}
