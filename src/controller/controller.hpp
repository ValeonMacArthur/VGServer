//
// Created by III on 2025/7/27.
//

#pragma once

#include <uWebSockets/App.h>

using HttpResponse = uWS::HttpResponse<false>;
using HttpRequest = uWS::HttpRequest;

namespace controller {
    void helloHandler(HttpResponse *res, HttpRequest *req);
    void userHandler(HttpResponse *res, HttpRequest *req);

    void verifyCodeHandler(HttpResponse *res, HttpRequest *req);
    void loginAndRegisterHandler(HttpResponse *res, HttpRequest *req);
    void loginOut(HttpResponse *res, HttpRequest *req);

    void topicClassificationHandler(HttpResponse *res, HttpRequest *req);
    void categoriesHandler(HttpResponse *res, HttpRequest *req);
    void searchShortDramaHandler(HttpResponse *res, HttpRequest *req);          // 搜索短剧
    void getProfileHandler(HttpResponse *res, HttpRequest *req);

    // 任务福利
    void rewordTasksHandler(HttpResponse *res, HttpRequest *req);
    void GetWelfareRewardHandler(HttpResponse *res, HttpRequest *req);

    void handleUnlockVideoWithGold(HttpResponse *res, HttpRequest *req);
    // 现金充值记录
    void GetUserPurchaseHandler(HttpResponse *res, HttpRequest *req);
    void UserPurchaseHandler(HttpResponse *res, HttpRequest *req);

    // 查询金币余额
    void handleGetGoldBalanceHandler(HttpResponse *res, HttpRequest *req);
    // 查询金币变动记录
    void handleGetGoldTransactionListHandler(HttpResponse *res, HttpRequest *req);
    // 订单相关
    void handleGetUserOrdersHandler(HttpResponse *res, HttpRequest *req);
    void handleCreateRechargeOrderHandler(HttpResponse *res, HttpRequest *req);

    // 消息相关
    void getUserMessagesHandler(HttpResponse *res, HttpRequest *req);
    void systemMessageHandler(HttpResponse *res, HttpRequest *req);


    // 获取活动列表
    void getCampaignListHandler(HttpResponse *res, HttpRequest *req);
    // 参加活动
    void joinCampaignHandler(HttpResponse *res, HttpRequest *req);
    // 活动详情
    void getCampaignDetailHandler(HttpResponse *res, HttpRequest *req);

    void getWatchHistoryHandler(HttpResponse *res, HttpRequest *req);        // 获取观看历史
    void getWatchlistHandler(HttpResponse *res, HttpRequest *req);           // 获取追剧列表
    void addToWatchlistHandler(HttpResponse *res, HttpRequest *req);         // 加入追剧
    void removeFromWatchlistHandler(HttpResponse *res, HttpRequest *req);

    void submitUserFeedbackHandler(HttpResponse *res, HttpRequest *req);
    void getUserFeedbackListHandler(HttpResponse *res, HttpRequest *req);


}