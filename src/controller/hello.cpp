//
// Created by III on 25-8-17.
//

#include "hello.hpp"
#include "pub.hpp"

// struct User {
//     int id;
//     std::string name;
// };
//
// // 提供序列化函数（必须）
// inline void to_json(nlohmann::json& j, const User& u) {
//     j = nlohmann::json{{"id", u.id}, {"name", u.name}};
// }
//
// inline void from_json(const nlohmann::json& j, User& u) {
//     j.at("id").get_to(u.id);
//     j.at("name").get_to(u.name);
// }
// void helloHandler(uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
//     User user{1, "Mike"};
//     sendJson(res, 200, "ok", user); // 自动调用 to_json
// }
void helloHandler(uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
    sendJson(res, 200, "Hello from controller!",nullptr);
}
