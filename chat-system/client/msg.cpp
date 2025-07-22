#include "msg.hpp"
#include "json.hpp"

using json = nlohmann::json;

std::vector<json> global_friend_requests;
std::mutex friend_requests_mutex;

// 处理服务端返回的错误消息
void error_msg(int fd, const json &response) {
    
    
    std::cerr << "[ERROR] 服务器返回错误: " << response.value("msg", "未知错误") << std::endl;
    
    
    // 可以做其他错误处理，比如弹窗、日志记录等
}

// 处理服务端返回的登录响应
bool log_in_msg(const json &response,std::string& token) {
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[登录成功] " << msg << std::endl;
        std::string tokenn = response.value("token", "");
        if (!tokenn.empty()) {
            // 保存token，供后续请求使用
            token=tokenn;
            // std::cout << "收到Token: " << tokenn << std::endl;
        }
        return true;
    } else if (status == "fail") {
        std::cout << "[登录失败] " << msg << std::endl;
        return false;
    } else {
        std::cerr << "[登录错误] " << msg << std::endl;
        return false;
    }
}

// 处理服务端返回的注册响应
void sign_up_msg(const json &response) {
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[注册成功] " << msg << std::endl;
        // 注册成功后，可以自动登录或提示用户去登录
    } else if (status == "fail") {
        std::cout << "[注册失败] " << msg << std::endl;
    } else {
        std::cerr << "[注册错误] " << msg << std::endl;
    }
}

void destory_account_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[注销成功] " << msg << std::endl;
        
    } else if (status == "fail") {
        std::cout << "[注销失败] " << msg << std::endl;
    } else {
        std::cerr << "[注册错误] " << msg << std::endl;
    }

}

void quit_account_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[退出成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[退出失败] " << msg << std::endl;
    } else {
        std::cerr << "[退出错误] " << msg << std::endl;
    }

}

void username_view_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");
    std::string username = response.value("username", "error");

    if (status == "success") {
        std::cout << "[查询成功] 帐号名为 :" << username << std::endl;
    } else if (status == "fail") {
        std::cout << "[查询失败] " << msg << std::endl;
    } else {
        std::cerr << "[查询错误] " << msg << std::endl;
    }

}

void username_change_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[修改成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[修改失败] " << msg << std::endl;
    } else {
        std::cerr << "[修改错误] " << msg << std::endl;
    }

}
void password_change_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[修改成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[修改失败] " << msg << std::endl;
    } else {
        std::cerr << "[修改错误] " << msg << std::endl;
    }

}

void show_friend_list_msg(const json &response){
    std::string status = response.value("status", "error");
    if (status == "success") {
        // nlohmann::json 支持隐式把 STL 容器转换为 JSON 数组
        auto friends = response["friends"];
        for (const auto& f : friends) {
            std::string account = f["account"];
            std::string username = f["username"];
            bool is_online = f["online"];
            bool is_muted = f["muted"];
            std::cout << username << " (" << account << ") is "
            << (is_online ? " online" : "offline")
            << (is_muted ? " [muted]" : "") << std::endl;
        }
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[列出错误] " << msg << std::endl;
    }
}

void add_friend_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[发送请求成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[发送请求失败] " << msg << std::endl;
    } else {
        std::cerr << "[发送请求错误] " << msg << std::endl;
    }
}

void remove_friend_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[删除成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[删除失败] " << msg << std::endl;
    } else {
        std::cerr << "[删除错误] " << msg << std::endl;
    }
}

void mute_friend_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[屏蔽成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[屏蔽失败] " << msg << std::endl;
    } else {
        std::cerr << "[屏蔽错误] " << msg << std::endl;
    }
}

void unmute_friend_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[解除屏蔽成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[解除屏蔽失败] " << msg << std::endl;
    } else {
        std::cerr << "[解除屏蔽错误] " << msg << std::endl;
    }
}

void get_friend_requests_msg(const json &response){
    std::lock_guard<std::mutex> lock(friend_requests_mutex);
    global_friend_requests.clear();
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[好友申请列表获取成功] " << msg << std::endl;
        auto requests = response["requests"];
        if (requests.empty()) {
            std::cout << "暂无待处理的好友请求" << std::endl;
        }else{

            int index = 0;
            for (const auto& r : requests) {
                global_friend_requests.push_back(r);
                std::cout << ++index << ". 用户名: " << r["username"]
                        << "(账号: " << r["account"] << ")" << std::endl;
            }

        }
    } else if (status == "fail") {
        std::cout << "[好友列表获取失败] " << msg << std::endl;
    } else {
        std::cerr << "[好友列表获取错误] " << msg << std::endl;
    }
}

void handle_friend_request_msg(const json &response){
    std::string status = response.value("status", "");
    std::string msg = response.value("msg", "未知错误");
    if (status == "success") {
        std::cout << "处理成功: " << msg << std::endl;
    } else {
        std::cout << "处理失败: " << msg << std::endl;
    }
}







void show_friend_notifications_msg(const json &response){
    
    
    
    
    
    ;
}








// resp["type"] = type;
// resp["status"] = "error";
// resp["msg"] = "Invalid or expired token.";