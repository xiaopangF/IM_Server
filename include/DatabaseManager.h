#pragma once
#include <string>
#include <mariadb/mysql.h>

class DatabaseManager {
public:
    // 获取单例实例（保证全程序只有一个数据库连接）
    static DatabaseManager& getInstance();

    // 连接数据库
    bool connect(const std::string& host, const std::string& user, 
                 const std::string& password, const std::string& dbname, int port);

    // 执行 SQL（增删改），返回是否成功
    bool execute(const std::string& sql);

    // 查询 SQL，返回结果集（由调用者负责 mysql_free_result）
    MYSQL_RES* query(const std::string& sql);

    // 获取最近一次错误信息
    std::string getLastError();

    // 析构时自动关闭连接
    ~DatabaseManager();

    // 执行预编译查询，返回 MYSQL_STMT*（调用者需要自己 fetch 和 close）
    MYSQL_STMT* prepareStatement(const std::string& sql);

private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    MYSQL* conn_ = nullptr;
};