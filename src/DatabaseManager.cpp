#include "DatabaseManager.h"
#include <iostream>

// 单例：返回全局唯一的实例
DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

// 连接数据库
bool DatabaseManager::connect(const std::string& host, const std::string& user,
                              const std::string& password, const std::string& dbname, int port) {
    if (conn_ != nullptr) {
        mysql_close(conn_);
    }
    conn_ = mysql_init(nullptr);
    if (conn_ == nullptr) {
        std::cerr << "❌ mysql_init 失败" << std::endl;
        return false;
    }
    if (!mysql_real_connect(conn_, host.c_str(), user.c_str(), password.c_str(),
                            dbname.c_str(), port, nullptr, 0)) {
        std::cerr << "❌ 连接失败: " << mysql_error(conn_) << std::endl;
        return false;
    }
    std::cout << "✅ 数据库连接成功！" << std::endl;
    return true;
}

// 执行 SQL（增删改）
bool DatabaseManager::execute(const std::string& sql) {
    if (conn_ == nullptr) return false;
    if (mysql_query(conn_, sql.c_str()) != 0) {
        std::cerr << "❌ 执行失败: " << mysql_error(conn_) << std::endl;
        return false;
    }
    return true;
}

// 查询 SQL
MYSQL_RES* DatabaseManager::query(const std::string& sql) {
    if (conn_ == nullptr) return nullptr;
    if (mysql_query(conn_, sql.c_str()) != 0) {
        std::cerr << "❌ 查询失败: " << mysql_error(conn_) << std::endl;
        return nullptr;
    }
    return mysql_store_result(conn_);
}

// 获取最近一次错误
std::string DatabaseManager::getLastError() {
    if (conn_ == nullptr) return "未连接";
    return mysql_error(conn_);
}

// 析构：自动关闭连接
DatabaseManager::~DatabaseManager() {
    if (conn_ != nullptr) {
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

MYSQL_STMT* DatabaseManager::prepareStatement(const std::string& sql) {
    if (conn_ == nullptr) return nullptr;

    MYSQL_STMT* stmt = mysql_stmt_init(conn_);
    if (stmt == nullptr) {
        std::cerr << "❌ mysql_stmt_init 失败" << std::endl;
        return nullptr;
    }

    if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
        std::cerr << "❌ mysql_stmt_prepare 失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }

    return stmt;
}