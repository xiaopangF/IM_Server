#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include "json.hpp"

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

using namespace std;
using json = nlohmann::json;

// ========== 打包：JSON -> [4字节长度][JSON字符串] ==========
static string encodePacket(const json& j) {
    string body = j.dump();
    uint32_t len = static_cast<uint32_t>(body.length());
    string packet;
    packet.append(reinterpret_cast<const char*>(&len), 4);
    packet.append(body);
    return packet;
}

// ========== 解包：读取服务端响应 ==========
static json recvResponse(int sockFd) {
    uint32_t bodyLen = 0;
    int n = recv(sockFd, reinterpret_cast<char*>(&bodyLen), 4, 0);
    if (n != 4) {
        throw runtime_error("读取长度头失败");
    }

    string body(bodyLen, '\0');
    int totalRead = 0;
    while (totalRead < static_cast<int>(bodyLen)) {
        n = recv(sockFd, &body[totalRead], bodyLen - totalRead, 0);
        if (n <= 0) throw runtime_error("读取 JSON 体失败");
        totalRead += n;
    }

    return json::parse(body);
}

// ========== 发送请求并接收响应 ==========
static json sendRequest(int sockFd, const json& req) {
    string packet = encodePacket(req);
    send(sockFd, packet.c_str(), packet.size(), 0);
    return recvResponse(sockFd);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // Windows 下初始化 Winsock
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "❌ Winsock 初始化失败" << endl;
        return -1;
    }
#endif

    // 1. 创建 Socket
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1) {
        cout << "❌ 创建 Socket 失败" << endl;
        return -1;
    }

    // 2. 连接服务端
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockFd, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        cout << "❌ 连接服务端失败，请确认服务端已启动" << endl;
#ifdef _WIN32
        closesocket(sockFd);
#else
        close(sockFd);
#endif
        return -1;
    }

    cout << "✅ 已连接到 IM 服务端 (127.0.0.1:8888)" << endl;

    // 3. 测试 ping
    {
        json req;
        req["type"] = "ping";
        req["seq"] = 1;
        json resp = sendRequest(sockFd, req);
        cout << "📡 ping 响应: " << resp.dump() << endl;
    }

    // 4. 测试注册
    {
        json req;
        req["type"] = "register";
        req["seq"] = 2;
        req["data"]["username"] = "client_user1";
        req["data"]["password"] = "123456";
        json resp = sendRequest(sockFd, req);
        cout << "📝 注册响应: " << resp.dump() << endl;
    }

    // 5. 测试登录
    {
        json req;
        req["type"] = "login";
        req["seq"] = 3;
        req["data"]["username"] = "client_user1";
        req["data"]["password"] = "123456";
        json resp = sendRequest(sockFd, req);
        cout << "🔑 登录响应: " << resp.dump() << endl;
    }

    // 6. 关闭连接
#ifdef _WIN32
    closesocket(sockFd);
    WSACleanup();
#else
    close(sockFd);
#endif
    cout << "🔌 连接已关闭" << endl;
    return 0;
}