#include "msg.hpp"
#include "json.hpp"

using json = nlohmann::json;

std::mutex io_mutex;


// 初始化
// 用来判断用户所在的界面 记录用户在和谁私聊
std::string current_chat_target = "";

// 用来知道非阻塞线程操作的哪个好友
std::vector<json> global_friend_requests;
std::mutex friend_requests_mutex;




// 用来判断用户所在的界面 记录用户在哪个群
std::string current_chat_group = "";
std::vector<json> global_group_requests;
std::mutex group_requests_mutex;






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

void get_friend_info_msg(const json &response){
    std::string status = response.value("status", "error");
    if (status == "success") {
            json friends = response["friend_info"];
            std::string account = friends["account"];
            std::string username = friends["username"];
            bool is_online = friends["online"];
            bool is_muted = friends["muted"];
            std::cout << username << " (" << account << ") is "
            << (is_online ? " online" : "offline")
            << (is_muted ? " [muted]" : "") << std::endl;
    }else if(status=="fail"){
        std::string msg = response.value("msg", "未知错误");
        std::cout <<"[查询失败] " << msg << std::endl;
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[查询错误] " << msg << std::endl;
    }
}

























void receive_private_message_msg(const json &response) {
    std::string from = response["from"];
    std::string message = response["message"];
    bool muted = response["muted"];

    if (muted) {
        // 屏蔽发送者，忽略消息
        return;
    }

    std::lock_guard<std::mutex> lock(io_mutex);

    // 保存当前输入文本和光标位置
    int saved_point = rl_point;

    // 保存当前 readline 输入行内容
    char* saved_line = rl_copy_text(0, rl_end);

    // 清除当前行并把光标移到行首
    rl_replace_line("", 0);
    std::cout << "\33[2K\r";

    // 打印新消息
    if (from == current_chat_target) {
        std::cout << "[" << from << "]: > " << message << std::endl;
    } else {
        std::cout << "[新消息来自 " << from << "]: " << message << std::endl;
        // 这里你可以做未读提醒等
    }

    // 恢复之前的用户输入
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    free(saved_line);

    // 通知 readline 新行开始，重新绘制输入行
    rl_on_new_line();
    rl_redisplay();
}

void get_private_history_msg(const json &response){
    std::string status = response.value("status", "error");
    if (status == "success") {
        auto message = response.value("messages",json::array());
        // auto message = response["message"];
        std::cout << "------- 最新历史记录 -------" << std::endl;
        if(message.empty()){
            std::cout<< "暂无历史记录"<<std::endl;
            std::cout << "--------------------------" << std::endl;
            return;
        }

        for(const auto&f : message){
            std::string from = f.value("from","");
            std::string to = f.value("to","");
            std::string content = f.value("content","");
            std::string timestamp = f.value("timestamp","");
            // std::cout << "[" << from << "]: " << " > "<<content << " " << timestamp << std::endl;
            
            std::cout << "[" << from << "]: " << " > "<< content << std::endl;
        }
        std::cout << "--------------------------" << std::endl;
    }else if(status=="fail"){
        std::string msg = response.value("msg", "未知错误");
        std::cout <<"[历史记录查询失败] " << msg << std::endl;
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[历史记录查询错误] " << msg << std::endl;
    }
}

void send_private_message_msg(const json &response){
    std::string status = response.value("status", "error");
    if(status =="success"){
        ;
    }else if(status=="fail"){
        std::string msg = response.value("msg", "未知错误");
        std::cout <<"[发送失败] " << msg << std::endl;
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[发送错误] " << msg << std::endl;
    }
}


void get_unread_private_messages_msg(const json &response){
        std::string status = response.value("status", "error");
    if (status == "success") {
        auto messages = response.value("messages",json::array());
        std::cout << "------- 离线消息 -------" << std::endl;
        if(messages.empty()){
            std::cout<< "暂无离线消息"<<std::endl;
            std::cout << "--------------------------" << std::endl;
            return;
        }

        for(const auto&f : messages){
            std::string from = f.value("from","");
            std::string content = f.value("content","");
            std::string timestamp = f.value("timestamp","");
            // std::cout << "[" << from << "]: " << " > "<<content << " " << timestamp << std::endl;
            std::cout << "[" << from << "]: " << " > "<<content << std::endl;
        }
        std::cout << "--------------------------" << std::endl;
    }else if(status=="fail"){
        std::string msg = response.value("msg", "未知错误");

        if(msg=="This user is not your friend"){
            std::cout <<"[非好友不能私聊] " << msg << std::endl;
            return;
        }
        
        if(msg=="Friend user not found"){
            std::cout <<"[根本没有这个用户] " << msg << std::endl;
            return;
        }
        std::cout <<"[离线消息查询失败] " << msg << std::endl;
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[离线消息查询错误] " << msg << std::endl;
    }
}

























void show_group_list_msg(const json &response){
        std::string status = response.value("status", "error");
    if (status == "success") {
        auto groups = response["groups"];
        for (const auto& f : groups) {
            std::string group_id = f["group_id"];
            std::string owner = f["owner"];
            std::string group_name = f["group_name"];
            std::cout <<  "[" << group_id << "] " <<group_name << " (owner: " << owner << ")"<< std::endl;
        }
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[列出错误] " << msg << std::endl;
    }
}















// resp["type"] = type;
// resp["status"] = "error";
// resp["msg"] = "Invalid or expired token.";