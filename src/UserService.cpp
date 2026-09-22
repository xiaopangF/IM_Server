#include "UserService.h"
#include <iostream>
#include <functional> // 用于 std::hash

// 简单哈希（⚠️ 不安全，仅用于跑通流程，后面会升级为 SHA-256）
std::string UserService::hashPassword(const std::string& password) {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(password);
    return std::to_string(hashValue);
}

bool UserService::registerUser(const std::string& username, const std::string& password, std::string& errorMsg) {
    // 1. 参数校验
    if (username.empty() || password.empty()) {
        errorMsg = "用户名或密码不能为空";
        return false;
    }
    if (username.length() > 50) {
        errorMsg = "用户名长度不能超过 50 个字符";
        return false;
    }
    if (password.length() < 6) {
        errorMsg = "密码长度不能少于 6 位";
        return false;
    }

    // 2. 检查用户名是否已存在
    auto existing = userDAO_.findUserByUsername(username);
    if (existing != nullptr) {
        errorMsg = "用户名已被注册";
        return false;
    }

    // 3. 哈希密码后写入数据库
    std::string passwordHash = hashPassword(password);
    if (!userDAO_.insertUser(username, passwordHash)) {
        errorMsg = "数据库写入失败";
        return false;
    }

    return true;
}

std::shared_ptr<User> UserService::login(const std::string& username, const std::string& password, std::string& errorMsg) {
    // 1. 查询用户
    auto user = userDAO_.findUserByUsername(username);
    if (user == nullptr) {
        errorMsg = "用户名或密码错误"; // 不提示“用户不存在”，防止被撞库
        return nullptr;
    }

    // 2. 比对密码哈希
    std::string inputHash = hashPassword(password);
    if (inputHash != user->getPasswordHash()) {
        errorMsg = "用户名或密码错误";
        return nullptr;
    }

    // 3. 登录成功
    return user;
}