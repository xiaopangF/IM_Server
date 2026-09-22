#include "UserDAO.h"
#include "DatabaseManager.h"
#include <mariadb/mysql.h>
#include <iostream>
#include <cstring>

bool UserDAO::insertUser(const std::string& username, const std::string& passwordHash) {
    auto& db = DatabaseManager::getInstance();

    // 用 ? 作为占位符
    std::string sql = "INSERT INTO users (username, password_hash) VALUES (?, ?);";
    MYSQL_STMT* stmt = db.prepareStatement(sql);
    if (stmt == nullptr) return false;

    // 准备参数
    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));

    // 参数 1：username
    unsigned long usernameLen = username.length();
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (void*)username.c_str();
    params[0].buffer_length = usernameLen;
    params[0].length = &usernameLen;

    // 参数 2：passwordHash
    unsigned long hashLen = passwordHash.length();
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (void*)passwordHash.c_str();
    params[1].buffer_length = hashLen;
    params[1].length = &hashLen;

    // 绑定参数
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        std::cerr << "❌ 绑定参数失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    // 执行
    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "❌ 执行失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

std::shared_ptr<User> UserDAO::findUserByUsername(const std::string& username) {
    auto& db = DatabaseManager::getInstance();

    // 用 ? 作为占位符
    std::string sql = "SELECT id, username, password_hash FROM users WHERE username = ? LIMIT 1;";
    MYSQL_STMT* stmt = db.prepareStatement(sql);
    if (stmt == nullptr) return nullptr;

    // 绑定参数
    MYSQL_BIND param;
    memset(&param, 0, sizeof(param));
    unsigned long usernameLen = username.length();
    param.buffer_type = MYSQL_TYPE_STRING;
    param.buffer = (void*)username.c_str();
    param.buffer_length = usernameLen;
    param.length = &usernameLen;

    if (mysql_stmt_bind_param(stmt, &param) != 0) {
        std::cerr << "❌ 绑定参数失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }

    // 执行
    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "❌ 执行失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }

    // 准备接收结果
    uint64_t id = 0;
    char nameBuf[64] = {0};
    char hashBuf[256] = {0};
    unsigned long nameLen = 0, hashLen = 0;

    MYSQL_BIND result[3];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &id;

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = nameBuf;
    result[1].buffer_length = sizeof(nameBuf);
    result[1].length = &nameLen;

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = hashBuf;
    result[2].buffer_length = sizeof(hashBuf);
    result[2].length = &hashLen;

    if (mysql_stmt_bind_result(stmt, result) != 0) {
        std::cerr << "❌ 绑定结果失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }

    // 取一行
    int fetchRet = mysql_stmt_fetch(stmt);
    if (fetchRet != 0 && fetchRet != MYSQL_DATA_TRUNCATED) {
        mysql_stmt_close(stmt);
        return nullptr; // 没查到
    }

    std::shared_ptr<User> user = std::make_shared<User>(id, std::string(nameBuf, nameLen), std::string(hashBuf, hashLen));
    mysql_stmt_close(stmt);
    return user;
}

std::shared_ptr<User> UserDAO::findUserById(uint64_t userId) {
    auto& db = DatabaseManager::getInstance();

    std::string sql = "SELECT id, username, password_hash FROM users WHERE id = ? LIMIT 1;";
    MYSQL_STMT* stmt = db.prepareStatement(sql);
    if (stmt == nullptr) return nullptr;

    MYSQL_BIND param;
    memset(&param, 0, sizeof(param));
    param.buffer_type = MYSQL_TYPE_LONGLONG;
    param.buffer = &userId;

    if (mysql_stmt_bind_param(stmt, &param) != 0 ||
        mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return nullptr;
    }

    uint64_t id = 0;
    char nameBuf[64] = {0};
    char hashBuf[256] = {0};
    unsigned long nameLen = 0, hashLen = 0;

    MYSQL_BIND result[3];
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &id;
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = nameBuf;
    result[1].buffer_length = sizeof(nameBuf);
    result[1].length = &nameLen;
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = hashBuf;
    result[2].buffer_length = sizeof(hashBuf);
    result[2].length = &hashLen;

    if (mysql_stmt_bind_result(stmt, result) != 0) {
        mysql_stmt_close(stmt);
        return nullptr;
    }

    int fetchRet = mysql_stmt_fetch(stmt);
    if (fetchRet != 0 && fetchRet != MYSQL_DATA_TRUNCATED) {
        mysql_stmt_close(stmt);
        return nullptr;
    }

    std::shared_ptr<User> user = std::make_shared<User>(id, std::string(nameBuf, nameLen), std::string(hashBuf, hashLen));
    mysql_stmt_close(stmt);
    return user;
}