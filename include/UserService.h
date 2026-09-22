#pragma once
#include <string>
#include <memory>
#include "UserDAO.h"

// 用户业务逻辑类
class UserService {
public:
    // 注册新用户，成功返回 true，失败返回 false 并设置 errorMsg
    bool registerUser(const std::string& username, const std::string& password, std::string& errorMsg);

    // 用户登录，成功返回 User 对象，失败返回 nullptr
    std::shared_ptr<User> login(const std::string& username, const std::string& password, std::string& errorMsg);

private:
    UserDAO userDAO_;

    // 计算密码的哈希值（后期会换成 SHA-256 加盐）
    std::string hashPassword(const std::string& password);
};