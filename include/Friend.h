#pragma once
#include <cstdint>
#include <string>

class Friend {
public:
    Friend() = default;
    Friend(uint64_t userId, uint64_t friendId, int status)
        : userId_(userId), friendId_(friendId), status_(status) {}

    uint64_t getUserId() const { return userId_; }
    uint64_t getFriendId() const { return friendId_; }
    int getStatus() const { return status_; }

private:
    uint64_t userId_ = 0;
    uint64_t friendId_ = 0;
    int status_ = 0; // 0:待验证, 1:已通过
};