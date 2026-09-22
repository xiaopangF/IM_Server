#include "SocketServer.h"
#include "DatabaseManager.h"
#include "UserService.h"
#include "FriendService.h"
#include "json.hpp"

#include <iostream>
#include <cstring>
#include <thread>

// Windows / Linux 跨平台网络头文件
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

using json = nlohmann::json;

// 跨平台关闭 socket 的辅助函数
static void closeSocket(int fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

SocketServer::SocketServer(int port)
    : port_(port), listenFd_(-1), running_(false) {}

SocketServer::~SocketServer() {
    stop();
}

void SocketServer::stop() {
    running_ = false;
    if (listenFd_ != -1) {
        closeSocket(listenFd_);
        listenFd_ = -1;
    }
}

void SocketServer::start() {
    // Windows 下必须先初始化 Winsock
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "❌ Winsock 初始化失败" << std::endl;
        return;
    }
#endif

    // 1. 创建 Socket
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ == -1) {
        std::cerr << "❌ 创建 Socket 失败" << std::endl;
        return;
    }

    // 2. 设置端口复用（防止重启时端口被占用）
    int opt = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    // 3. 绑定端口
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<u_short>(port_));

    if (bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        std::cerr << "❌ 绑定端口 " << port_ << " 失败" << std::endl;
        closeSocket(listenFd_);
        listenFd_ = -1;
        return;
    }

    // 4. 开始监听
    if (listen(listenFd_, 128) == -1) {
        std::cerr << "❌ 监听失败" << std::endl;
        closeSocket(listenFd_);
        listenFd_ = -1;
        return;
    }

    std::cout << "🚀 IM Server 已启动，监听端口 " << port_ << std::endl;
    running_ = true;

    // 5. 循环接受客户端连接
    while (running_) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif

        int clientFd = accept(listenFd_, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientFd == -1) {
            if (running_) std::cerr << "⚠️ accept 失败" << std::endl;
            continue;
        }

        std::string clientIp = inet_ntoa(clientAddr.sin_addr);
        std::cout << "👤 新客户端连接: " << clientIp << " (fd=" << clientFd << ")" << std::endl;

        // 6. 为每个客户端启动一个新线程
        std::thread(&SocketServer::handleClient, this, clientFd, clientIp).detach();
    }
}

void SocketServer::handleClient(int clientFd, std::string clientIp) {
    UserService userService;
    FriendService friendService;

    while (running_) {
        // 1. 读取 4 字节长度头
        uint32_t bodyLen = 0;
        int n = recv(clientFd, reinterpret_cast<char*>(&bodyLen), 4, 0);
        if (n != 4) {
            std::cout << "👋 客户端断开: " << clientIp << std::endl;
            break;
        }

        // 2. 读取 JSON 体
        std::string body(bodyLen, '\0');
        int totalRead = 0;
        while (totalRead < static_cast<int>(bodyLen)) {
            n = recv(clientFd, &body[totalRead], bodyLen - totalRead, 0);
            if (n <= 0) {
                std::cout << "⚠️ 读取 JSON 数据中断" << std::endl;
                closeSocket(clientFd);
                return;
            }
            totalRead += n;
        }

        // 3. 解析 JSON
        json request;
        try {
            request = json::parse(body);
        } catch (const std::exception& e) {
            std::cerr << "❌ JSON 解析失败: " << e.what() << std::endl;
            break;
        }

        // 4. 分发业务逻辑
        json response;
        response["type"] = request.value("type", "unknown");
        response["seq"] = request.value("seq", 0);

        try {
            std::string type = request.value("type", "");

            if (type == "ping") {
                response["code"] = 200;
                response["msg"] = "pong";

            } else if (type == "register") {
                std::string username = request["data"].value("username", "");
                std::string password = request["data"].value("password", "");
                std::string errMsg;
                if (userService.registerUser(username, password, errMsg)) {
                    response["code"] = 200;
                    response["msg"] = "注册成功";
                } else {
                    response["code"] = 400;
                    response["msg"] = errMsg;
                }

            } else if (type == "login") {
                std::string username = request["data"].value("username", "");
                std::string password = request["data"].value("password", "");
                std::string errMsg;
                auto user = userService.login(username, password, errMsg);
                if (user) {
                    response["code"] = 200;
                    response["msg"] = "登录成功";
                    response["data"]["userId"] = user->getId();
                    response["data"]["username"] = user->getUsername();
                } else {
                    response["code"] = 400;
                    response["msg"] = errMsg;
                }

            } else if (type == "add_friend") {
                uint64_t userId = request["data"].value("userId", 0);
                uint64_t friendId = request["data"].value("friendId", 0);
                std::string errMsg;
                if (friendService.sendRequest(userId, friendId, errMsg)) {
                    response["code"] = 200;
                    response["msg"] = "好友申请已发送";
                } else {
                    response["code"] = 400;
                    response["msg"] = errMsg;
                }

            } else if (type == "accept_friend") {
                uint64_t userId = request["data"].value("userId", 0);
                uint64_t friendId = request["data"].value("friendId", 0);
                std::string errMsg;
                if (friendService.acceptRequest(userId, friendId, errMsg)) {
                    response["code"] = 200;
                    response["msg"] = "已同意好友申请";
                } else {
                    response["code"] = 400;
                    response["msg"] = errMsg;
                }

            } else {
                response["code"] = 404;
                response["msg"] = "未知的请求类型: " + type;
            }

        } catch (const std::exception& e) {
            response["code"] = 500;
            response["msg"] = std::string("服务端异常: ") + e.what();
        }

        // 5. 打包响应并发回客户端
        std::string respBody = response.dump();
        uint32_t respLen = static_cast<uint32_t>(respBody.length());

        send(clientFd, reinterpret_cast<const char*>(&respLen), 4, 0);
        send(clientFd, respBody.c_str(), respLen, 0);
    }

    closeSocket(clientFd);
}