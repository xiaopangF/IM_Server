#include "FriendService.h"
#include <iostream>

bool FriendService::sendRequest(uint64_t userId, uint64_t friendId, std::string& errorMsg) {
    // 1. 不能加自己为好友
    if (userId == friendId) {
        errorMsg = "不能添加自己为好友";
        return false;
    }

    // 2. 已经是好友，不能再申请
    if (friendDAO_.isFriend(userId, friendId)) {
        errorMsg = "你们已经是好友了";
        return false;
    }

    // 3. 发送申请（插入一条记录，status=0）
    if (!friendDAO_.sendFriendRequest(userId, friendId)) {
        errorMsg = "发送好友申请失败，可能已申请过";
        return false;
    }

    return true;
}

bool FriendService::acceptRequest(uint64_t userId, uint64_t friendId, std::string& errorMsg) {
    // 1. 检查是否已经是好友
    if (friendDAO_.isFriend(userId, friendId)) {
        errorMsg = "你们已经是好友了";
        return false;
    }

    // 2. 更新申请状态为“已通过”
    if (!friendDAO_.acceptFriendRequest(userId, friendId)) {
        errorMsg = "同意申请失败，可能对方没有向你发送申请";
        return false;
    }

    return true;
}

std::vector<uint64_t> FriendService::getFriendList(uint64_t userId) {
    return friendDAO_.getFriendList(userId);
}

bool FriendService::isFriend(uint64_t userId, uint64_t friendId) {
    return friendDAO_.isFriend(userId, friendId);
}