#include <iostream>
#include <windows.h>
#include "DatabaseManager.h"
#include "UserService.h"

using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // 1. 连接数据库
    auto& db = DatabaseManager::getInstance();
    if (!db.connect("localhost", "root", "jiaoyixuan1001", "im_server", 3306)) {
        cout << "❌ 数据库连接失败" << endl;
        return -1;
    }

    UserService userService;

    // 2. 测试注册
    string errMsg;
    bool ok = userService.registerUser("bob", "123456", errMsg);
    cout << "注册结果: " << (ok ? "✅ 成功" : "❌ 失败: " + errMsg) << endl;

    // 3. 测试重复注册
    errMsg.clear();
    ok = userService.registerUser("bob", "abcdef", errMsg);
    cout << "重复注册结果: " << (ok ? "✅ 成功" : "❌ 失败: " + errMsg) << endl;

    // 4. 测试登录（正确密码）
    errMsg.clear();
    auto user = userService.login("bob", "123456", errMsg);
    cout << "登录（正确密码）: " << (user ? "✅ 成功，用户ID=" + to_string(user->getId()) : "❌ 失败: " + errMsg) << endl;

    // 5. 测试登录（错误密码）
    errMsg.clear();
    user = userService.login("bob", "wrongpwd", errMsg);
    cout << "登录（错误密码）: " << (user ? "✅ 成功" : "❌ 失败: " + errMsg) << endl;

    return 0;
}