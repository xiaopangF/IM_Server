#pragma once
#include <string>
#include <cstdint>

// 用户实体类：对应数据库 users 表中的一行
class User {
public:
    User() = default;
    User(uint64_t id, const std::string& username, const std::string& passwordHash)
        : id_(id), username_(username), passwordHash_(passwordHash) {}

    // Getter 和 Setter
    uint64_t getId() const { return id_; }
    void setId(uint64_t id) { id_ = id; }

    std::string getUsername() const { return username_; }
    void setUsername(const std::string& u) { username_ = u; }

    std::string getPasswordHash() const { return passwordHash_; }
    void setPasswordHash(const std::string& p) { passwordHash_ = p; }

private:
    uint64_t id_ = 0;
    std::string username_;
    std::string passwordHash_;
};