#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "FriendDAO.h"

class FriendService {
public:
    // 发送好友申请
    // 返回 true 表示成功，false 表示失败，具体原因写在 errorMsg
    bool sendRequest(uint64_t userId, uint64_t friendId, std::string& errorMsg);

    // 同意好友申请
    bool acceptRequest(uint64_t userId, uint64_t friendId, std::string& errorMsg);

    // 查询好友列表
    std::vector<uint64_t> getFriendList(uint64_t userId);

    // 检查两人是否是好友
    bool isFriend(uint64_t userId, uint64_t friendId);

private:
    FriendDAO friendDAO_;
};