#include "UserDAO.h"
#include "DatabaseManager.h"
#include <mariadb/mysql.h>
#include <iostream>
#include <sstream>

bool UserDAO::insertUser(const std::string& username, const std::string& passwordHash) {
    auto& db = DatabaseManager::getInstance();

    // ⚠️ 拼 SQL 是不安全的（SQL注入），但为了现阶段跑通，先用这个方式
    // 后续我们会用 mysql_real_escape_string 或者 prepared statement 来防注入
    std::string sql = "INSERT INTO users (username, password_hash) VALUES ('"
                      + username + "', '" + passwordHash + "');";

    return db.execute(sql);
}

std::shared_ptr<User> UserDAO::findUserByUsername(const std::string& username) {
    auto& db = DatabaseManager::getInstance();
    std::string sql = "SELECT id, username, password_hash FROM users WHERE username = '"
                      + username + "' LIMIT 1;";

    MYSQL_RES* res = db.query(sql);
    if (res == nullptr) return nullptr;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr) {
        mysql_free_result(res);
        return nullptr; // 没找到用户
    }

    uint64_t id = std::stoull(row[0]);
    std::string name = row[1];
    std::string hash = row[2];

    mysql_free_result(res);

    return std::make_shared<User>(id, name, hash);
}

std::shared_ptr<User> UserDAO::findUserById(uint64_t userId) {
    auto& db = DatabaseManager::getInstance();
    std::string sql = "SELECT id, username, password_hash FROM users WHERE id = "
                      + std::to_string(userId) + " LIMIT 1;";

    MYSQL_RES* res = db.query(sql);
    if (res == nullptr) return nullptr;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr) {
        mysql_free_result(res);
        return nullptr;
    }

    uint64_t id = std::stoull(row[0]);
    std::string name = row[1];
    std::string hash = row[2];

    mysql_free_result(res);

    return std::make_shared<User>(id, name, hash);
}