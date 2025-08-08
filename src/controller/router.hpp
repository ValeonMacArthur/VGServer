//
// Created by III on 2025/7/27.
//
#pragma once

#include <uWebSockets/App.h>
#include "controller.hpp"
#include "chat_ws_controller.hpp"

inline void setup_routes(uWS::App &app) {
    app
    .get("/hello", controller::helloHandler)
    .get("/user/:id", controller::userHandler)
    .get("/v2/user/:id", controller::verifyCodeHandler)
    //反馈
    .ws<UserData>("/chat/*", {
        .open = chat_on_open,
        .message = chat_on_message,
        .close = chat_on_close,})
    //人工
    .ws<UserData>("/support", {/* 你的 ws 配置 */});
}