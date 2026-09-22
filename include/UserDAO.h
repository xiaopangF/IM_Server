#pragma once
#include <string>
#include <memory>
#include "User.h"

// 用户数据访问对象：专门负责 users 表的增删改查
class UserDAO {
public:
    // 插入新用户，成功返回 true
    bool insertUser(const std::string& username, const std::string& passwordHash);

    // 根据用户名查询用户，如果找到返回 User 对象，找不到返回 nullptr
    std::shared_ptr<User> findUserByUsername(const std::string& username);

    // 根据用户 ID 查询用户名
    std::shared_ptr<User> findUserById(uint64_t userId);
};