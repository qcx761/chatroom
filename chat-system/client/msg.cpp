#include "msg.hpp"
#include "json.hpp"

using json = nlohmann::json;

std::mutex io_mutex;



// 用来判断用户所在的界面 记录用户在和谁私聊
std::string current_chat_target = "";
// 用来知道非阻塞线程操作的哪个好友
std::vector<json> global_friend_requests;
std::mutex friend_requests_mutex;

// 用来判断用户所在的界面 记录用户在哪个群
std::string current_chat_group = "";
std::vector<json> global_group_requests;
std::mutex group_requests_mutex;

std::unordered_map<std::string,bool> friend_bemuted_map;
std::vector<std::string> file_list;
std::vector<std::string> friend_list;
std::vector<std::string> group_list;

// 处理服务端返回的错误消息
void error_msg(int fd, const json &response) {
    std::cerr << "[ERROR] 服务器返回错误: " << response.value("msg", "未知错误") << std::endl;
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

        friend_list.clear();
        // nlohmann::json 支持隐式把 STL 容器转换为 JSON 数组

        auto friends = response["friends"];
        for (const auto& f : friends) {
            std::string account = f["account"];
            std::string username = f["username"];

            friend_list.push_back(username);

            bool is_online = f["online"];
            bool is_muted = f["muted"];




            bool is_bemuted = f["bemuted"];
            friend_bemuted_map[username] = is_bemuted;





            //在发送文件时我拉取好友列表但是不能输出
            if(current_chat_target == ""){

            std::cout << "用户名:"<< username << " 帐号:" << account << " is "
            << (is_online ? " online" : "offline")
            << (is_muted ? " [muted]" : "") << std::endl;
            }
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
            std::cout << "用户名:"<< username << " 帐号:" << account << " is "
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

    // 打印新消息
    if (from == current_chat_target) {
        std::cout << "[" << from << "]: > " << message << std::endl;
    } else {
            std::lock_guard<std::mutex> lock(io_mutex);

    // 保存当前输入文本和光标位置
    int saved_point = rl_point;

    // 保存当前 readline 输入行内容
    char* saved_line = rl_copy_text(0, rl_end);

    // 清除当前行并把光标移到行首
    rl_replace_line("", 0);
    std::cout << "\33[2K\r";
        std::cout << "[新消息来自 " << from << "]: " << message << std::endl;
        // std::cout << "[新消息来自 " << from << "]" << std::endl;
        // 这里你可以做未读提醒等

            // 恢复之前的用户输入
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    free(saved_line);

    // 通知 readline 新行开始，重新绘制输入行
    rl_on_new_line();
    rl_redisplay();
    }
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

        group_list.clear();

        auto groups = response["groups"];
        for (const auto& f : groups) {
            int group_id = f["group_id"];
            std::string owner = f["owner"];
            std::string group_name = f["group_name"];

            group_list.push_back(group_name);

            std::string role = f["role"];

            if(current_chat_group == "")
            std::cout <<  "id:" << group_id <<"  name:"<< group_name << "  owner:" << owner << "  role:" << role << std::endl;
        }
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[列出错误] " << msg << std::endl;
    }
}

void join_group_msg(const json &response){
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

void quit_group_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[退出群聊成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[退出群聊失败] " << msg << std::endl;
    } else {
        std::cerr << "[退出群聊错误] " << msg << std::endl;
    }
}

void show_group_members_msg(const json &response){
    std::string status = response.value("status", "error");
    if (status == "success") {
        // nlohmann::json 支持隐式把 STL 容器转换为 JSON 数组
        std::string group_name = response["group_name"];
        auto members = response["members"];
        for (const auto& m : members) {
            std::string account = m["account"];
            std::string name = m["name"];
            std::string role = m["role"];
            std::cout << "name:" << name << " account:" << account  
            << " role:"<< role << " in " << group_name <<std::endl;
        }
    } else if (status == "fail") {
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[列出失败] " << msg << std::endl;
    } else {
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[列出错误] " << msg << std::endl;
    }
}


void create_group_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[创建群聊成功] " << msg << std::endl;
        // 可以输出id
    } else if (status == "fail") {
        std::cout << "[创建群聊失败] " << msg << std::endl;
    } else {
        std::cerr << "[创建群聊错误] " << msg << std::endl;
    }
}

void set_group_admin_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[添加管理员成功] " << msg << std::endl;
        // 可以输出id
    } else if (status == "fail") {
        std::cout << "[添加管理员失败] " << msg << std::endl;
    } else {
        std::cerr << "[添加管理员错误] " << msg << std::endl;
    }
}

void remove_group_admin_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[删除管理员成功] " << msg << std::endl;
        // 可以输出id
    } else if (status == "fail") {
        std::cout << "[删除管理员失败] " << msg << std::endl;
    } else {
        std::cerr << "[删除管理员错误] " << msg << std::endl;
    }
}

void remove_group_member_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[删除成员成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[删除成员失败] " << msg << std::endl;
    } else {
        std::cerr << "[删除成员错误] " << msg << std::endl;
    }
}

void add_group_member_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[添加成员成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[添加成员失败] " << msg << std::endl;
    } else {
        std::cerr << "[添加成员错误] " << msg << std::endl;
    }
}

void dismiss_group_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[解散成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[解散失败] " << msg << std::endl;
    } else {
        std::cerr << "[解散错误] " << msg << std::endl;
    }
}

void get_group_requests_msg(const json &response){
    std::lock_guard<std::mutex> lock(group_requests_mutex);
    global_group_requests.clear();
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        // std::cout << "[群申请列表获取成功] " << msg << std::endl;
        auto requests = response["requests"];
        if (requests.empty()) {
            std::cout << "暂无待处理的群申请请求" << std::endl;
        }else{
            int index = 0;
            for (const auto& r : requests) {
                global_group_requests.push_back(r);
                std::string username = r.value("username", "");
                std::string account = r.value("account", "");
                std::string group_name = r.value("group_name", "");
                std::cout << ++index << ". 用户名:" << username
                << " 账号:" << account << " from " 
                << group_name << std::endl;
            }
        }
    } else if (status == "fail") {
        std::cout << "[群申请列表获取失败] " << msg << std::endl;
    } else {
        std::cerr << "[群申请列表获取错误] " << msg << std::endl;
    }
}

void handle_group_request_msg(const json &response){
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[处理成功] " << msg << std::endl;
    } else if (status == "fail") {
        std::cout << "[处理失败] " << msg << std::endl;
    } else {
        std::cerr << "[处理错误] " << msg << std::endl;
    }
}

void get_unread_group_messages_msg(const json &response){
    std::string status = response.value("status", "error");
    if (status == "success") {
        auto messages = response.value("messages",json::array());
        std::cout << "------- 未读消息及上次聊天记录 -------" << std::endl;
        if(messages.empty()){
            std::cout<< "暂无未读消息"<<std::endl;
            std::cout << "--------------------------" << std::endl;
            return;
        }

        for(const auto&f : messages){
            std::string from = f.value("sender","");
            std::string content = f.value("content","");
            std::string timestamp = f.value("timestamp","");
            std::cout << "[" << from << "]: " << " > "<<content << std::endl;
        }
        std::cout << "--------------------------" << std::endl;
    }else if(status=="fail"){
        std::string msg = response.value("msg", "未知错误");
        std::cout <<"[未读消息查询失败] " << msg << std::endl;
    }else{
        std::string msg = response.value("msg", "未知错误");
        std::cerr << "[未读消息查询错误] " << msg << std::endl;
    }
}

void receive_group_messages_msg(const json &response){
    std::string from = response["from"];
    std::string message = response["message"];
    std::string group_name = response["group_name"];

    std::lock_guard<std::mutex> lock(io_mutex);
    // 打印新消息
    if (group_name == current_chat_group) {
        std::cout << "[" << group_name << "] " << from <<":"<< message << std::endl;
    } else {

    // 保存当前输入文本和光标位置
    int saved_point = rl_point;

    // 保存当前 readline 输入行内容
    char* saved_line = rl_copy_text(0, rl_end);

    // 清除当前行并把光标移到行首
    rl_replace_line("", 0);
    std::cout << "\33[2K\r";
    std::cout << "[新消息来自 " << from << "] in " << group_name << std::endl;

    // 恢复之前的用户输入
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    free(saved_line);

    // 通知 readline 新行开始，重新绘制输入行
    rl_on_new_line();
    rl_redisplay();
    }
    }

void get_group_history_msg(const json &response){
        std::string group_name = response.value("group_name", "");
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
            std::string from = f.value("sender","");
            std::string content = f.value("content","");
            std::string timestamp = f.value("timestamp","");
            
            std::cout << "[" << group_name << "] " << from <<":"<< content << std::endl;
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

void send_group_message_msg(const json &response){
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

void receive_message_msg(const json &response){
    std::string status = response.value("status", "error");
    if(status =="fail"){
        ;
    }

    std::lock_guard<std::mutex> lock(io_mutex);

    // 保存当前输入文本和光标位置
    int saved_point = rl_point;

    // 保存当前 readline 输入行内容
    char* saved_line = rl_copy_text(0, rl_end);

    // 清除当前行并把光标移到行首
    rl_replace_line("", 0);
    std::cout << "\33[2K\r";

    std::string msg = response.value("type1", "");
    if(msg=="private_file_message"){
    std::string name = response.value("from", "");

        std::cout << "收到来自 " << name << "的文件传输" << std::endl;
    }

    if(msg=="group_file_message"){
    std::string name = response.value("from", "");
    std::string group = response.value("group", "");

        std::cout << "收到来自 " << group << "群中 "<< name << "的文件传输" << std::endl;
    }

    if(msg=="add_friend_message"){
    std::string name = response.value("user_name", "");

    std::cout << "收到来自 " << name << "的好友请求" << std::endl;
    }

    if(msg=="pass_friend_message"){
    std::string name = response.value("user_name", "");

    std::cout << name << " 已经通过你的的好友请求" << std::endl;
    }

    if(msg=="add_group_message"){
    std::string name = response.value("name", "");
    int id = response.value("id", 0);


    std::cout << "收到来自 " << name << "的加群请求 群id:" << id << std::endl;
    }

    if(msg=="pass_group_message"){
    int id = response.value("id", 0);

    std::cout << "你的加群id:" << id << " 请求已被通过" << id << std::endl;
    }


    // 恢复之前的用户输入
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    free(saved_line);

    // 通知 readline 新行开始，重新绘制输入行
    rl_on_new_line();
    rl_redisplay();
}


void get_file_list_msg(const json &response){
    std::string status = response.value("status", "error");
    file_list.clear();
    if (status == "success") {
        auto list = response.value("files",json::array());
        std::cout << "------- 文件接收 -------" << std::endl;
        if(list.empty()){
            std::cout<< "暂无文件"<<std::endl;
            std::cout << "--------------------------" << std::endl;
            return;
        }

        for(const auto&l : list){
            std::string type = l.value("type","");
            if(type == "private"){
                std::string sender = l.value("sender","");
                std::string filename = l.value("filename","");
                std::string time = l.value("timestamp","");
                std::cout << "来自用户帐号为:" << sender << " 的文件 " << filename << " 时间：" << time << std::endl;
                std::string word ="p" + sender + filename +time;
                file_list.push_back(word);
                // file["type"] = "private";
                // file["id"] = res->getInt("id");
                // file["sender"] = res->getString("sender");
                // file["filename"] = res->getString("filename");
                // file["filesize"] = res->getString("filesize");
                // file["filepath"] = res->getString("filepath");
                // file["timestamp"] = res->getString("timestamp");
            }

            if(type == "group"){
                std::string sender = l.value("sender","");
                std::string filename = l.value("filename","");
                int id = l.value("group_id",0);
                std::string time = l.value("timestamp","");
                std::cout << "来自用户帐号为:" << sender << " 的文件 " << filename << " 在群id为"<< id << " 时间：" << time << std::endl;
                std::string word ="g" + sender + filename + time;
                file_list.push_back(word);
                // file["type"] = "group";
                // file["id"] = res->getInt("id");
                // file["group_id"] = res->getInt("group_id");
                // file["sender"] = res->getString("sender");
                // file["filename"] = res->getString("filename");
                // file["filesize"] = res->getString("filesize");
                // file["filepath"] = res->getString("filepath");
                // file["timestamp"] = res->getString("timestamp");
            }
        }
        std::cout << "--------------------------" << std::endl;

    }else if(status=="fail"){
        std::cout <<"[文件查询失败] " << std::endl;
    }else{
        std::cerr << "[文件查询错误] " << std::endl;
    }
}

// 登录发送摘要
void offline_summary_msg(const json &response) {
    if (response.contains("status") && response["status"] == "success") {
        std::cout << "\n你有新的离线消息摘要:\n";

        if (response.contains("messages") && response["messages"].is_array()) {
            const auto& messages = response["messages"];

            if (messages.empty()) {
                std::cout << "没有新的离线消息。\n";
                return;
            }

            for (const auto& msg : messages) {
                std::string sender = msg.value("sender", "未知发送者");
                std::string type = msg.value("message_type", "未知类型");
                int count = msg.value("count", 0);

                std::string type_str;
                if (type == "private_text") type_str = "私聊";
                else if (type == "private_file") type_str = "私聊文件";
                else if (type == "group_text") type_str = "群聊";
                else if (type == "group_file") type_str = "群聊文件";
                else type_str = "未知";

                std::cout << " 来自 [" << sender << "] 的 " << type_str
                          << " 消息，共 " << count << " 条。\n";
            }
        } else {
            std::cout << "服务器返回了空的消息列表。\n";
        }
    } else {
        std::cout << "获取离线摘要失败：" << response.value("msg", "未知错误") << std::endl;
    }
}





