#include <iostream>
#include <windows.h>
#include "DatabaseManager.h"
#include "SocketServer.h"

using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // 1. 连接数据库
    auto& db = DatabaseManager::getInstance();
    if (!db.connect("localhost", "root", "jiaoyixuan1001", "im_server", 3306)) {
        cout << "❌ 数据库连接失败" << endl;
        return -1;
    }

    // 2. 启动 Socket 服务器
    SocketServer server(8888);
    server.start();  // 阻塞式循环

    return 0;
}