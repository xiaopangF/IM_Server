#pragma once
#include <cstdint>
#include <vector>
#include "Friend.h"

class FriendDAO {
public:
    // 发送好友申请（A -> B, status = 0）
    bool sendFriendRequest(uint64_t userId, uint64_t friendId);

    // 同意好友申请（把 A->B 的 status 改为 1）
    bool acceptFriendRequest(uint64_t userId, uint64_t friendId);

    // 查询某个用户的好友列表
    std::vector<uint64_t> getFriendList(uint64_t userId);

    // 检查两人是否是好友
    bool isFriend(uint64_t userId, uint64_t friendId);
};