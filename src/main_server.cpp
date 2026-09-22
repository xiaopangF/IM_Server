#include <iostream>
#include <windows.h>
#include "DatabaseManager.h"
#include "UserDAO.h"

using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // 1. 连接数据库
    auto& db = DatabaseManager::getInstance();
    if (!db.connect("localhost", "root", "jiaoyixuan1001", "im_server", 3306)) {
        cout << "❌ 数据库连接失败" << endl;
        return -1;
    }

    // 2. 测试插入用户
    UserDAO userDAO;
    bool ok = userDAO.insertUser("alice", "hash_password_123");
    cout << (ok ? "✅ 用户 alice 插入成功" : "❌ 用户 alice 插入失败") << endl;

    // 3. 测试查询用户
    auto user = userDAO.findUserByUsername("alice");
    if (user) {
        cout << "🔍 查询成功！" << endl;
        cout << "   ID: " << user->getId() << endl;
        cout << "   Username: " << user->getUsername() << endl;
        cout << "   PasswordHash: " << user->getPasswordHash() << endl;
    } else {
        cout << "❌ 未找到用户 alice" << endl;
    }

    return 0;
}