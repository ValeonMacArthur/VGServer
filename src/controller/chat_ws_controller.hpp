//
// Created by III on 2025/7/28.
//

#pragma once

#include <uWebSockets/App.h>

struct UserData {
    int userId;
    // 其他字段
};

void chat_on_open(uWS::WebSocket<false, true, UserData>* ws);
void chat_on_message(uWS::WebSocket<false, true, UserData>* ws, std::string_view msg, uWS::OpCode opCode);
void chat_on_close(uWS::WebSocket<false, true, UserData>* ws, int code, std::string_view message);