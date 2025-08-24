#ifdef _WIN32
#define UNICODE
#define _UNICODE
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#warning "This code may not work on Linux or other environments."

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#endif

#include "csc_basic.h"

// 函数声明
void broadcastMessage(const std::string &message);
void sendAdminMessage(const std::string &message);
void kickUser(const std::string &identifier);
void runServerCommands();
bool initializeNetwork();
void cleanupNetwork();

std::vector<SOCKET> clients;
std::mutex clients_mutex;
std::map<SOCKET, std::pair<std::string, int>> clientInfo; // 存储用户名和 ID
int nextClientId = 1;                                     // 用于分配唯一 ID

const std::string VERSION = "1.2";
const std::string SERVER_NAME = "Cloud Studio Chat";

// 获取在线人数
int getOnlineFriends()
{
    std::lock_guard<std::mutex> guard(clients_mutex);
    return clients.size();
}

void broadcastMessage(const std::string &message)
{
    // 在服务端控制台输出消息
    std::cout << message << "\n>  " << std::flush;
    std::lock_guard<std::mutex> guard(clients_mutex);
    for (SOCKET client : clients)
    {
        send(client, message.c_str(), message.size(), 0);
    }
}

void listOnlineFriends(SOCKET clientSocket)
{
    std::lock_guard<std::mutex> guard(clients_mutex);
    std::string message = "Online Users:\n";
    for (const auto &client : clientInfo)
    {
        message += "Username: " + client.second.first + ", ID: " + std::to_string(client.second.second) + ", Socket: " + std::to_string(client.first) + "\n";
    }
    send(clientSocket, message.c_str(), message.size(), 0);
}

void kickUser(const std::string &identifier)
{
    std::lock_guard<std::mutex> guard(clients_mutex);
    for (auto it = clientInfo.begin(); it != clientInfo.end(); ++it)
    {
        if (it->second.first == identifier || std::to_string(it->second.second) == identifier)
        {
            SOCKET clientSocket = it->first;
            send(clientSocket, "You have been kicked out by the admin.\n", 43, 0);
            closesocket(clientSocket);
            clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
            clientInfo.erase(it);
            break;
        }
    }
}

void sendAdminMessage(const std::string &message)
{
    std::string adminMessage = "{[Server] Admin}: " + message;
    broadcastMessage(adminMessage);
}

void handleClient(SOCKET clientSocket)
{
    char buffer[32768];
    int bytesReceived;

    std::string username;
    // 首次接收用户名
    bytesReceived = recv(clientSocket, buffer, 32768, 0);
    if (bytesReceived <= 0)
    {
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';
    username = buffer;
    // 去除末尾换行和回车
    while (!username.empty() && (username.back() == '\n' || username.back() == '\r'))
    {
        username.pop_back();
    }

    {
        std::lock_guard<std::mutex> guard(clients_mutex);
        clientInfo[clientSocket] = {username, nextClientId++};
    }

    // 用户进入时在服务端输出，并且广播
    broadcastMessage(username + " has joined.");

    while ((bytesReceived = recv(clientSocket, buffer, 32768, 0)) > 0)
    {
        buffer[bytesReceived] = '\0';
        std::string message(buffer);
        // 去除末尾换行和回车
        while (!message.empty() && (message.back() == '\n' || message.back() == '\r'))
        {
            message.pop_back();
        }

        if (message.empty())
        {
            continue;
        }

        if (message == "/list")
        {
            listOnlineFriends(clientSocket);
        }
        else if (message.rfind("/kick ", 0) == 0)
        {
            std::string identifier = message.substr(6);
            kickUser(identifier);
        }
        else if (message.rfind("/adminmsg ", 0) == 0)
        {
            std::string adminMessage = message.substr(10);
            sendAdminMessage(adminMessage);
        }
        else
        {
            broadcastMessage(message);
        }
    }

    // 客户端断开连接
    std::string disconnected_username;
    {
        std::lock_guard<std::mutex> guard(clients_mutex);
        auto it = clientInfo.find(clientSocket);
        if (it != clientInfo.end())
        {
            disconnected_username = it->second.first;
            clientInfo.erase(it);
        }
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }
    closesocket(clientSocket);

    if (!disconnected_username.empty())
    {
        broadcastMessage(disconnected_username + " has left.");
    }
}

bool initializeNetwork()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed." << std::endl;
        return false;
    }
#endif
    return true;
}

void cleanupNetwork()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

void runServerCommands()
{
    std::string cmd;
    while (true)
    {
        std::cout << ">  " << std::flush;
        std::getline(std::cin, cmd);
        if (cmd == "list")
        {
            std::lock_guard<std::mutex> guard(clients_mutex);
            std::cout << "Online Users:" << std::endl;
            for (const auto &client : clientInfo)
            {
                std::cout << "Username: " << client.second.first
                          << ", ID: " << client.second.second
                          << ", Socket: " << client.first << std::endl;
            }
        }
        else if (cmd.rfind("kick ", 0) == 0)
        {
            std::string identifier = cmd.substr(5);
            kickUser(identifier);
            std::cout << "Kick command sent for: " << identifier << std::endl;
        }
        else if (cmd.rfind("adminmsg ", 0) == 0)
        {
            std::string msg = cmd.substr(9);
            sendAdminMessage(msg);
            std::cout << "Admin message sent." << std::endl;
        }
        else if (cmd == "helpqwq")
        {
            std::cout << "Commands:\n"
                      << "list                  - Show online users\n"
                      << "kick <name|id>        - Kick user by name or id\n"
                      << "adminmsg <message>    - Broadcast admin message\n"
                      << "helpqwq               - Show this help\n"
                      << "exit/quit             - Shut down the server\n"
                      << "about                 - Show about message.\n";
        }
        else if (cmd == "exit" || cmd == "quit")
        {
            broadcastMessage("Server is closed.");
            std::cout << "Shutting down server..." << std::endl;
            cleanupNetwork();
            exit(0);
        }
        else if (cmd.rfind("about", 0) == 0)
        {
            std::wcout << client_name << std::wstring(L"\nVersion: ") << VERSION << std::endl;
        }
        else if (!cmd.empty())
        {
            std::cout << "Unknown command: " << cmd << std::endl
                      << "Use 'helpqwq' to view help" << std::endl;
        }
    }
}

int main()
{
    if (!initializeNetwork())
    {
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed." << std::endl;
        cleanupNetwork();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6543);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        cleanupNetwork();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        cleanupNetwork();
        return 1;
    }

    std::cout << "Server started on port 6543" << std::endl;

    // 服务端命令线程
    std::thread(runServerCommands).detach();

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> guard(clients_mutex);
            clients.push_back(clientSocket);
        }

        std::thread(handleClient, clientSocket).detach();
    }

    closesocket(serverSocket);
    cleanupNetwork();
    return 0;
}