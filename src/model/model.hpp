//
// Created by III on 2025/7/28.
//

#pragma once

#include <optional>
#include <string>
#include <vector>

enum class VideoStatus {
    DRAFT,
    PUBLISHED,
    OFFLINE
};


struct Person {
    std::string user_id;
    std::string nickname;
    std::string avatar_url;
    int level = 1;
    bool is_vip = false;
    std::optional<std::string> vip_expire_at;  // 可为空
    int coin_balance = 0;
    std::string register_channel;
    // 新增
    std::optional<std::string> phone_number;   // 手机号，唯一索引
    std::optional<std::string> email;          // 邮箱，唯一索引

    std::string created_at;
};

struct Series {
    std::string series_id;
    std::string title;
    std::string cover_url;
    std::string description;
    std::string category_id;
    std::vector<std::string> tags;  // 标签支持多个
    int episode_count = 0;
    int free_episodes = 0;
    bool is_vip_only = false;
    std::string created_at;
};

struct Video {
    std::string video_id;
    std::string title;
    std::string description;
    std::string play_url;
    std::string cover_url;
    std::vector<std::string> tags;
    std::string category_id;
    std::optional<std::string> series_id;
    std::optional<int> episode_index;
    bool is_vip_only = false;
    int coin_price = 0;
    int play_count = 0;
    int like_count = 0;
    int reward_count = 0;
    std::string status = "draft";  // 或 enum 类型
    std::string created_at;
};

struct Reward {
    std::string reward_id;
    std::string user_id;
    std::string video_id;
    std::string gift_type;   // 如 "火箭"、"玫瑰"
    int gift_value = 0;
    std::string created_at;
};

//消费类型
enum class ItemType {
    VIDEO,
    VIP,
    GIFT,
    COIN
};

struct Purchase {
    std::string purchase_id;
    std::string user_id;
    std::string item_type;   // "video", "vip", "gift"
    std::string item_id;
    int coin_amount = 0;
    std::string created_at;
};

struct WatchHistory {
    long id;
    std::string user_id;
    std::string video_id;
    int progress_sec = 0;
    std::string watched_at;
};

