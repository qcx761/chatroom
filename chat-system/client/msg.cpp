#include"msg.hpp"
#include"json.hpp"



// 处理服务端接收并且发送信息

void error_msg(int fd,const json &request){
    json response;
    response["type"] = "error";
    response["msg"] = "Unrecognized request type";
    
    send_json(fd, response);

}



void log_in_msg(int fd, const json &request){

    // std::string type=request.value("type","");
    // json response;

    // std::string username = request.value("username", "");
    // std::string password = request.value("password", "");

    // if(username.empty() || password.empty()){
    //     response["type"] = "log_in";
    //     response["status"] = "error";
    //     response["msg"] = "username or password is empty";
    //     send_json(fd, response);
        // continue;
    // }

    // try {
    //     auto redis = Redis("tcp://127.0.0.1:6379");

    //     if (!redis.exists("user:" + username)) {
    //         response["type"] = "log_in";
    //         response["status"] = "fail";
    //         response["msg"] = "User not found";
    //     } else {
    //         auto stored_pass = redis.hget("user:" + username, "password");
    //         if (stored_pass && *stored_pass == password) {
    //             response["type"] = "log_in";
    //             response["status"] = "success";
    //             response["msg"] = "Login successful";
    //         } else {
    //             response["type"] = "log_in";
    //             response["status"] = "fail";
    //             response["msg"] = "Incorrect password";
    //         }
    //     }
    // } catch (const Error &e) {
    //     response["type"] = "log_in";
    //     response["status"] = "error";
    //     response["msg"] = std::string("Redis error: ") + e.what();
    // }

    // send_json(fd, response);
}



void sign_up_msg(int fd, const json &request){

    // std::string type=request.value("type","");
    // json response;

    // std::string username = request.value("username", "");
    // std::string password = request.value("password", "");

    // if(username.empty() || password.empty()){
    //     response["type"] = "sign_up";
    //     response["status"] = "error";
    //     response["msg"] = "username or password is empty";
    //     send_json(fd, response);
    //     // continue;
    // }

    // try {
    //     // 连接 Redis（可以提取为类的成员变量，不建议每次都重新连接）
    //     auto redis = Redis("tcp://127.0.0.1:6379");

    //     // 检查是否已存在
    //     if (redis.exists("user:" + username)) {
    //         response["type"] = "sign_up";
    //         response["status"] = "fail";
    //         response["msg"] = "Username already exists";
    //     } else {
    //         // 注册成功，存储账号密码（明文存储仅用于示例，实际要哈希加密）
    //         redis.hset("user:" + username, "password", password);
    //         response["type"] = "sign_up";
    //         response["status"] = "success";
    //         response["msg"] = "Registered successfully";
    //     }
    // } catch (const Error &e) {
    //     response["type"] = "sign_up";
    //     response["status"] = "error";
    //     response["msg"] = std::string("Redis error: ") + e.what();
    // }

    // send_json(fd, response);
}
