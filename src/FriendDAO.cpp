#include "FriendDAO.h"
#include "DatabaseManager.h"
#include <mariadb/mysql.h>
#include <iostream>
#include <cstring>

bool FriendDAO::sendFriendRequest(uint64_t userId, uint64_t friendId) {
    auto& db = DatabaseManager::getInstance();
    std::string sql = "INSERT INTO friendships (user_id, friend_id, status) VALUES (?, ?, 0);";
    MYSQL_STMT* stmt = db.prepareStatement(sql);
    if (stmt == nullptr) return false;

    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &userId;
    params[1].buffer_type = MYSQL_TYPE_LONGLONG;
    params[1].buffer = &friendId;

    if (mysql_stmt_bind_param(stmt, params) != 0 ||
        mysql_stmt_execute(stmt) != 0) {
        std::cerr << "❌ 发送好友申请失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
}

bool FriendDAO::acceptFriendRequest(uint64_t userId, uint64_t friendId) {
    auto& db = DatabaseManager::getInstance();
    // 把 A->B 的申请状态改为 1
    std::string sql = "UPDATE friendships SET status = 1 WHERE user_id = ? AND friend_id = ? AND status = 0;";
    MYSQL_STMT* stmt = db.prepareStatement(sql);
    if (stmt == nullptr) return false;

    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &userId;
    params[1].buffer_type = MYSQL_TYPE_LONGLONG;
    params[1].buffer = &friendId;

    if (mysql_stmt_bind_param(stmt, params) != 0 ||
        mysql_stmt_execute(stmt) != 0) {
        std::cerr << "❌ 同意好友申请失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
}

std::vector<uint64_t> FriendDAO::getFriendList(uint64_t userId) {
    std::vector<uint64_t> friends;
    auto& db = DatabaseManager::getInstance();
    // 查询 user_id 是 A 且 status=1 的好友；同时也查 friend_id 是 A 且 status=1 的（因为好友关系是双向的）
    std::string sql = "SELECT friend_id FROM friendships WHERE user_id = ? AND status = 1 "
                      "UNION "
                      "SELECT user_id FROM friendships WHERE friend_id = ? AND status = 1;";
    MYSQL_STMT* stmt = db.prepareStatement(sql);
    if (stmt == nullptr) return friends;

    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &userId;
    params[1].buffer_type = MYSQL_TYPE_LONGLONG;
    params[1].buffer = &userId;

    if (mysql_stmt_bind_param(stmt, params) != 0 ||
        mysql_stmt_execute(stmt) != 0) {
        std::cerr << "❌ 查询好友列表失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return friends;
    }

    uint64_t friendId = 0;
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &friendId;

    if (mysql_stmt_bind_result(stmt, result) != 0) {
        mysql_stmt_close(stmt);
        return friends;
    }

    while (mysql_stmt_fetch(stmt) == 0) {
        friends.push_back(friendId);
    }

    mysql_stmt_close(stmt);
    return friends;
}

bool FriendDAO::isFriend(uint64_t userId, uint64_t friendId) {
    auto& db = DatabaseManager::getInstance();
    std::string sql = "SELECT 1 FROM friendships WHERE "
                      "((user_id = ? AND friend_id = ?) OR (user_id = ? AND friend_id = ?)) "
                      "AND status = 1 LIMIT 1;";
    MYSQL_STMT* stmt = db.prepareStatement(sql);
    if (stmt == nullptr) return false;

    MYSQL_BIND params[4];
    memset(params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &userId;
    params[1].buffer_type = MYSQL_TYPE_LONGLONG;
    params[1].buffer = &friendId;
    params[2].buffer_type = MYSQL_TYPE_LONGLONG;
    params[2].buffer = &friendId;
    params[3].buffer_type = MYSQL_TYPE_LONGLONG;
    params[3].buffer = &userId;

    if (mysql_stmt_bind_param(stmt, params) != 0 ||
        mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }

    int flag = 0;
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &flag;

    if (mysql_stmt_bind_result(stmt, result) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }

    bool found = (mysql_stmt_fetch(stmt) == 0);
    mysql_stmt_close(stmt);
    return found;
}