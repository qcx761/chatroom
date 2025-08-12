#include "account.hpp"
#include "json.hpp"
#include "msg.hpp"

std::string unique_account;

string readline_string(const string& prompt) {
    char* input = readline(prompt.c_str());
    if (!input) return "";
    string result(input);
    if (!result.empty()) add_history(input);
    free(input);
    return result;
}

void waiting() {
    readline_string("按 Enter 键继续...");
}

// void flushInput() {
//     cin.clear();
//     cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
// }

// 关闭终端回显，读一行密码
string get_password(const string& prompt) {

    struct termios oldt, newt;
    cout << prompt;

    // 获取当前终端属性
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // 关闭回显
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    string password;
    getline(cin, password);

    // 恢复终端属性
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << endl;

    return password;
}

void main_menu_ui(int sock,sem_t& sem,std::atomic<bool>& login_success) {
    while (!login_success.load()) {
        system("clear"); // 清屏
        show_main_menu();
        int n;

        string input = readline_string("请输入你的选项：");
        try {
            n = stoi(input);
        } catch (...) {
            cout << "无效的输入，请输入数字。" << endl;
            waiting();
            continue;
        }

        switch (n) {
        case 1:
            log_in(sock,sem);
            // flushInput();
            waiting();
            break;
        case 2:
            sign_up(sock,sem);
            // flushInput();
            waiting();
            break;
        case 3:
            exit(0);
        default:
            cout << "无效数字" << endl;
            waiting();
            break;
        }
    }
}

void log_in(int sock,sem_t& sem) {
    system("clear");
    cout << "登录" << endl;
    json j;
    j["type"] = "log_in";

    string account = readline_string("请输入帐号   :");
    string password = get_password("请输入密码   :");

    unique_account = account;

    j["account"] = account;
    j["password"] = password;
    send_json(sock, j);
    sem_wait(&sem); // 等待信号量
}

void sign_up(int sock,sem_t& sem) {
    system("clear");
    cout << "注册" << endl;
    json j;
    j["type"] = "sign_up";

    string username = readline_string("请输入用户名 :");
    string account = readline_string("请输入帐号  :");
    string password_old = get_password("请输入密码   :");
    string password_new = get_password("请再次输入密码:");

        if (password_new == password_old) {
            j["account"]  = account;
            j["username"] = username;
            j["password"] = password_old;
            send_json(sock, j);
            sem_wait(&sem); // 等待信号量
        } else {
            cout << "两次密码不一样" << endl;
        }
}

void destory_account(int sock,string token,sem_t& sem){
    system("clear");
    cout <<"注销函数 :" << endl;
    string a = readline_string("确定要注销帐号吗(Y/N) :");
    if(a == "Y" || a == "y"){
        string account = readline_string("请输入帐号  :");
        string password = get_password("请输入密码   :");
        json j;
        j["type"] = "destory_account";
        j["token"] = token;
        j["account"]=account;
        j["password"]=password;
        send_json(sock, j);
    }else{
        cout<< "已取消注销"<< endl;
        waiting();
        return;
    }
    sem_wait(&sem); // 等待信号量
    waiting();
}

void quit_account(int sock,string token,sem_t& sem){
    json j;
    j["type"]="quit_account";
    j["token"]=token;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    // flushInput();
    waiting();
}

void username_view(int sock,string token,sem_t& sem){
    json j;
    j["type"]="username_view";
    j["token"]=token;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    // flushInput();
    waiting();
}

void username_change(int sock,string token,sem_t& sem){
    system("clear");
    // cout <<"修改用户名 :" << endl;
    string username = readline_string("请输入想修改的用户名  :");
    json j;
    j["type"]="username_change";
    j["token"]=token;
    j["username"]=username;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    // flushInput();
    waiting();
}

void password_change(int sock,string token,sem_t& sem){
    system("clear");
    // cout <<"密码修改 :" << endl;
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string password_old = get_password("请输入旧密码   :");
    string password_new = get_password("请输入新密码   :");
    string password_new1 = get_password("请再次输入新密码:");

    if(password_new!=password_new1){
        cout<<"两次密码不一样"<<endl;
        waiting();
        return;
    }
    json j;
    j["type"]="password_change";
    j["token"]=token;
    j["old_password"]=password_old;
    j["new_password"]=password_new;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    // flushInput();
    waiting();
}

void show_friend_list(int sock,string token,sem_t& sem){
    system("clear");
    // cout <<"好友列表 :" << endl;
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');
    json j;
    j["type"]="show_friend_list";
    j["token"]=token;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    // flushInput();
    waiting();

}

void add_friend(int sock,string token,sem_t& sem){
    system("clear");
    // cout <<"添加用户 :" << endl;
    string target_username = readline_string("请输入想添加的用户名  :");
    json j;
    j["type"]="add_friend";
    j["token"]=token;
    j["target_username"]=target_username;

    send_json(sock,j);
    sem_wait(&sem);
    // flushInput();
    waiting();
}



void remove_friend(int sock,string token,sem_t& sem){
    system("clear");
    // cout <<"删除用户 :" << endl;
    cout << "---------- 好友列表 ----------" << endl;



    json mm;
    mm["type"]="show_friend_list";
    mm["token"]=token;
    send_json(sock,mm);
    sem_wait(&sem); // 等待信号量


    cout << "-----------------------------" << endl;
    string username = readline_string("请输入想删除的用户名  :");

    json j;
    j["type"]="remove_friend";
    j["token"]=token;
    j["target_username"]=username;

    send_json(sock,j);
    sem_wait(&sem);
    // flushInput();
    waiting();
}


void unmute_friend(int sock,string token,sem_t& sem){
    system("clear");
    // cout <<"解除屏蔽用户 :" << endl;
    cout << "---------- 好友列表 ----------" << endl;



    json mm;
    mm["type"]="show_friend_list";
    mm["token"]=token;
    send_json(sock,mm);
    sem_wait(&sem); // 等待信号量


    cout << "-----------------------------" << endl;
    string target_username = readline_string("请输入想解除屏蔽的用户名  :");


    json j;
    j["type"]="unmute_friend";
    j["token"]=token;
    j["target_username"]=target_username;

    send_json(sock,j);
    sem_wait(&sem);
    // flushInput();
    waiting();
}


void mute_friend(int sock,string token,sem_t& sem){
    system("clear");
    cout << "---------- 好友列表 ----------" << endl;



    json mm;
    mm["type"]="show_friend_list";
    mm["token"]=token;
    send_json(sock,mm);
    sem_wait(&sem); // 等待信号量


    cout << "-----------------------------" << endl;
    // cout <<"屏蔽用户 :" << endl;
    string target_username = readline_string("请输入想屏蔽的用户名  :");


    json j;
    j["type"]="mute_friend";
    j["token"]=token;
    j["target_username"]=target_username;

    send_json(sock,j);
    sem_wait(&sem);
    // flushInput();
    waiting();
}


void getandhandle_friend_request(int sock,string token,sem_t& sem){
    system("clear");
    std::cout << "好友请求列表" << std::endl;
    json j;
    j["type"]="get_friend_requests";
    j["token"]=token;
    send_json(sock,j);
    sem_wait(&sem);

    if(global_friend_requests.size()==0){
        // flushInput();
        waiting();
        return;
    }

    string input = readline_string("输入处理编号(0 退出): ");

    // int choice = stoi(input);
    int choice;
    try {
        choice = std::stoi(input);
    } catch (const std::exception& e) {
        std::cout << "输入错误已取消处理。" << std::endl;
        waiting();
        return;
    }


    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "输入错误已取消处理。" << std::endl;
        waiting();
        return;
    }

    from_username = global_friend_requests[choice - 1]["username"];
}
    string op = readline_string("你想如何处理 [" + from_username + "] 的请求？(a=接受, r=拒绝): ");
    // flushInput();

    std::string action;
    if (op == "a" || op == "A") {
        action = "accept";
    } else if (op == "r" || op == "R") {
        action = "reject";
    } else {
        std::cout << "无效操作，已取消处理。" << std::endl;
        waiting();
        return;
    }

    json m;
    m["type"] = "handle_friend_request";
    m["token"] = token;
    m["from_username"] = from_username;
    m["action"] = action;
    send_json(sock, m);
    sem_wait(&sem);

    // flushInput();
    waiting();
}

void get_friend_info(int sock, string token,sem_t& sem){
    system("clear");
    // cout <<"好友信息查询 :" << endl;
    string target_username = readline_string("输入查找的好友名 :");

    json j;
    j["type"]="get_friend_info";
    j["token"]=token;
    j["target_username"]=target_username;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    waiting();
}

void send_private_message(int sock,string token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;
    cout << "---------- 好友列表 ----------" << endl;
    json mm;
    mm["type"]="show_friend_list";
    mm["token"]=token;
    send_json(sock,mm);
    sem_wait(&sem); // 等待信号量
    cout << "-----------------------------" << endl;

    string target_username = readline_string("私聊的用户名 : ");

    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }

    bool found = false;
    for (const auto& list : friend_list) {
        if (list == target_username) {
            found = true;
            break;
        }
    }
    if(!found){
        cout<< "不是好友无法私聊" << endl;
        waiting();
        return;
    }



    // 设置当前私聊用户
    current_chat_target = target_username;

    // // 拉取离线消息
    // json req;
    // req["type"] = "get_unread_private_messages";
    // req["token"] = token;
    // req["target_username"] = target_username;
    // send_json(sock, req);
    // sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file [路径] 传输文件" << endl;
    cout << "- 输入 /exit 退出" << endl;
    cout << "- 输入 /help 获取提示" << endl;
    while (true) {
        // string message = readline_string("");
        // string message = readline_string("> ");
            // if (message.empty()) {
            //     continue;
            // }
        std::string message;   // 用来接收每一行的字符串变量

        std::getline(std::cin,message);
        // std::cout << message <<std::endl;

        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
        }

        if (message == "/help") {
            cout << "提示："<< endl;
            cout << "- 输入消息并回车发送" << endl;
            cout << "- 输入 /history [数量] 查看历史记录" << endl;
            cout << "- 输入 /file [路径] 传输文件" << endl;
            cout << "- 输入 /exit 退出" << endl;
            cout << "- 输入/help 获取提示" << endl;
            continue;
        }

        if (message.rfind("/history", 0) == 0) {
            int count = 10;
            std::istringstream iss_history(message);
            string cmd;
            iss_history >> cmd >> count;

            json history;
            history["type"]="get_private_history";
            history["token"]=token;
            history["target_username"]=target_username;
            history["count"]=count;
            send_json(sock, history);
            sem_wait(&sem);  
            continue;
        }

        if (message.rfind("/file", 0) == 0) {
json mmm;
mmm["type"]="show_friend_list";
mmm["token"]=token;
send_json(sock,mmm);
sem_wait(&sem); // 等待信号量
bool found1 = false;
for (const auto& list : friend_list) {
    if (list == target_username) {
        found1 = true;
        break;
    }
}
if(!found1){
    cout<< "不是好友无法发送文件" << endl;
    waiting();
    return;
}
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                waiting();
                continue;
            }

            // 判断是不是目录
            if (fs::is_directory(path)) {
                cout << "[错误] 不能传输目录！" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                waiting();
                continue;
            }

            // 提取文件名和大小
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) {
                cout << "[错误] 文件打开失败！" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                waiting();
                continue;
            }

            long long filesize_ll = file.tellg();
            std::string filesize = std::to_string(filesize_ll);
            // string filename = path.substr(path.find_last_of("/") + 1);



            // 修改filename作为 account+是否群消息(p/g)+filename+时间戳
            string filename1 = path.substr(path.find_last_of("/") + 1);
            string filename = "p" + unique_account + filename1;




            std::thread([sock, token, target_username , path, filename , filesize , filename1]() {
                const std::string ftp_ip = "10.30.1.215";
                const int ftp_port = 2100;

                int control_fd = connect_to_server(ftp_ip, ftp_port);
                if (control_fd < 0) {
                    std::cerr << "[错误] 连接FTP控制端失败\n" << endl;
                    waiting();
                    return;
                }

                ftp_stor(control_fd, filename, path);
                close(control_fd);
                // 文件发送成功告诉服务端推送文件
                json file_req;
                file_req["type"] = "send_private_file";
                file_req["token"] = token;
                file_req["target_username"] = target_username;
                file_req["filename"] = filename1;
                file_req["filesize"] = filesize;
                send_json(sock, file_req);

                // cout << "文件上传完成" << endl;
            }).detach();
            cout << "[系统] 文件上传后台进行中..." << endl;
            continue;
        }


        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;
        send_json(sock, msg);
    }
    current_chat_target = "";
}

void receive_file(int sock,string token,sem_t& sem){
    system("clear");
    json j;
    j["type"] = "get_file_list";
    j["token"] = token;
    send_json(sock, j);
    sem_wait(&sem);
    // string filename = readline_string("请输入想接收的文件名 : ");
    // string path1 = readline_string("请输入想放在的路径 : ");



    string filename1 = readline_string("请输入想接收的文件名 : ");
    if(filename1.empty()){
        std::cout << "输入错误" << std::endl;
        waiting();
        return;
    }
    if (!filename1.empty() && filename1.back() == '\n') {
        filename1.pop_back();  // 直接删掉最后一个字符
    }


    string typee = readline_string("请输入文件的类型(群文件:g / 个人文件:p) : ");
    if(typee.empty()){
        std::cout << "输入错误" << std::endl;
        waiting();
        return;
    }
    if (!typee.empty() && typee.back() == '\n') {
        typee.pop_back();  // 直接删掉最后一个字符
    }


    string account = readline_string("请输入文件发送者的帐号 : ");
    if(account.empty()){
        std::cout << "输入错误" << std::endl;
        waiting();
        return;
    }
    if (!account.empty() && account.back() == '\n') {
        account.pop_back();  // 直接删掉最后一个字符
    }

    // 2025-08-11 14:35:22
    string time = readline_string("请输入文件的时间戳（年-月-日 小时:分钟:秒） : ");
    if(time.empty()){
        std::cout << "输入错误" << std::endl;
        waiting();
        return;
    }
    if (!time.empty() && time.back() == '\n') {
        time.pop_back();  // 直接删掉最后一个字符
    }

    std::string filename = typee + account + filename1 + time;
    // std::string filename = typee + account + filename1;

    string path1 = readline_string("请输入想放在的路径（直接回车保存默认路径） : ");
    if (!path1.empty() && path1.back() == '\n') {
        // 删除换行符
        path1.pop_back();
    }

if (path1.empty()) {
    path1 = "./";  // 默认保存
}

if (path1.back() != '/'){
    path1 += '/';
}

    string path = path1 + filename1;

    std::thread([sock, token, path, filename]() {
        const std::string ftp_ip = "10.30.1.215";
        const int ftp_port = 2100;

        int control_fd = connect_to_server(ftp_ip, ftp_port);
        if (control_fd < 0) {
            std::cerr << "[错误] 连接FTP控制端失败\n" << endl;
            return;
        }
        ftp_retr(control_fd, filename, path);
        close(control_fd);

    // std::cout << "文件下载完成" << std::endl;

    }).detach();

    cout << "[系统] 文件下载已开始，后台进行中..." << endl;


    waiting();
}

void show_group_list(int sock, string token, sem_t &sem){
    system("clear");
    json j;
    j["type"] = "show_group_list";
    j["token"] = token;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void join_group(int sock,string token,sem_t& sem){
    system("clear");
    string group_name = readline_string("输入想加入的群聊名称 : ");
    json j;
    j["type"] = "join_group";
    j["token"] = token;
    j["group_name"] = group_name;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void quit_group(int sock,string token,sem_t& sem){
    system("clear");
    


    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;






    string group_name = readline_string("输入要退出的群聊名称 : ");
    json j;
    j["type"] = "quit_group";
    j["token"] = token;
    j["group_name"] = group_name;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void show_group_members(int sock,string token,sem_t& sem){
    system("clear");

    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    string group_name = readline_string("输入要查看成员的群聊名称 : ");
    json j;
    j["type"] = "show_group_members";
    j["token"] = token;
    j["group_name"] = group_name;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void create_group(int sock,string token,sem_t& sem){
    system("clear");
    string group_name = readline_string("请输入新建群聊名称 : ");
    json j;
    j["type"] = "create_group";
    j["token"] = token;
    j["group_name"] = group_name;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void set_group_admin(int sock,string token,sem_t& sem){
    system("clear");

    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    string group_name = readline_string("输入群聊名称 : ");

    bool found = false;
    for (const auto& list : group_list) {
        if (list == group_name) {
            found = true;
            break;
        }
    }
    if(!found){
        cout<< "未找到该群" << endl;
        waiting();
        return;
    }


    cout << "---------- 群成员列表 ----------" << endl;
    json jjj;
    jjj["type"] = "show_group_members";
    jjj["token"] = token;
    jjj["group_name"] = group_name;
    send_json(sock, jjj);
    sem_wait(&sem);
    cout << "-------------------------------" << endl;


    string target_user = readline_string("输入要设为管理员的用户名 : ");
    json j;
    j["type"] = "set_group_admin";
    j["token"] = token;
    j["group_name"] = group_name;
    j["target_name"] = target_user;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void remove_group_admin(int sock,string token,sem_t& sem){
    system("clear");

    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    string group_name = readline_string("输入群聊名称 : ");

    bool found = false;
    for (const auto& list : group_list) {
        if (list == group_name) {
            found = true;
            break;
        }
    }
    if(!found){
        cout<< "未找到该群" << endl;
        waiting();
        return;
    }

    cout << "---------- 群成员列表 ----------" << endl;
    json jjj;
    jjj["type"] = "show_group_members";
    jjj["token"] = token;
    jjj["group_name"] = group_name;
    send_json(sock, jjj);
    sem_wait(&sem);
    cout << "-------------------------------" << endl;

    string target_user = readline_string("输入要移除管理员权限的用户名 : ");
    json j;
    j["type"] = "remove_group_admin";
    j["token"] = token;
    j["group_name"] = group_name;
    j["target_name"] = target_user;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void remove_group_member(int sock,string token,sem_t& sem){
    system("clear");

    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    string group_name = readline_string("输入群聊名称 : ");

    bool found = false;
    for (const auto& list : group_list) {
        if (list == group_name) {
            found = true;
            break;
        }
    }
    if(!found){
        cout<< "未找到该群" << endl;
        waiting();
        return;
    }

    cout << "---------- 群成员列表 ----------" << endl;
    json jjj;
    jjj["type"] = "show_group_members";
    jjj["token"] = token;
    jjj["group_name"] = group_name;
    send_json(sock, jjj);
    sem_wait(&sem);
    cout << "-------------------------------" << endl;

    string target_user = readline_string("输入要移除的成员用户名 : ");
    json j;
    j["type"] = "remove_group_member";
    j["token"] = token;
    j["group_name"] = group_name;
    j["target_name"] = target_user;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void add_group_member(int sock,string token,sem_t& sem){
    system("clear");

    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    string group_name = readline_string("输入群聊名称 : ");
    string new_member = readline_string("输入新成员用户名 : ");
    json j;
    j["type"] = "add_group_member";
    j["token"] = token;
    j["group_name"] = group_name;
    j["new_member"] = new_member;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void dismiss_group(int sock,string token,sem_t& sem){
    system("clear");

    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    string group_name = readline_string("输入要解散的群聊名称 : ");
    json j;
    j["type"] = "dismiss_group";
    j["token"] = token;
    j["group_name"] = group_name;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void getandhandle_group_request(int sock,string token,sem_t& sem){
    system("clear");

    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    std::cout << "请求列表" << std::endl;
    string group_name = readline_string("输入查询的群名 : ");
    
    json j;
    j["type"]="get_group_requests";
    j["group_name"] =group_name;
    j["token"]=token;
    send_json(sock,j);
    sem_wait(&sem);

    if(global_group_requests.size()==0){
        waiting();
        return;
    }

    string input = readline_string("输入处理编号(0 退出): ");
    // int choice = stoi(input);
    int choice;
    try {
        choice = std::stoi(input);
    } catch (const std::exception& e) {
        std::cout << "输入错误已取消处理。" << std::endl;
        waiting();
        return;
    }

    std::string from_username;
    {
        std::lock_guard<std::mutex> lock(group_requests_mutex);

        if (choice <= 0 || choice > global_group_requests.size()) {
            std::cout << "已取消处理。" << std::endl;
            waiting();
            return;
        }

        from_username = global_group_requests[choice - 1]["username"];
    }
    string op = readline_string("你想如何处理 [用户名为" + from_username + "] 的请求？(a=接受, r=拒绝): ");

    std::string action;
    if (op == "a" || op == "A") {
        action = "accept";
    } else if (op == "r" || op == "R") {
        action = "reject";
    } else {
        std::cout << "无效操作，已取消处理。" << std::endl;
        waiting();
        return;
    }

    json g;
    g["type"] = "handle_group_request";
    g["token"] = token;
    g["from_username"] = from_username;
    g["action"] = action;
    g["group_name"] = group_name;
    send_json(sock, g);
    sem_wait(&sem);
    waiting();
}

void send_group_message(int sock,string token,sem_t& sem){
system("clear");
    cout << "========== 群聊 ==========" << endl;


    cout << "---------- 群列表 ----------" << endl;
    json jj;
    jj["type"] = "show_group_list";
    jj["token"] = token;
    send_json(sock, jj);
    sem_wait(&sem);
    cout << "-----------------------------" << endl;

    string target_group = readline_string("群聊名 : ");
    if (target_group.empty()) {
        cout << "[错误] 群名不能为空" << endl;
        return;
    }

    bool found = false;
    for (const auto& list : group_list) {
        if (list == target_group) {
            found = true;
            break;
        }
    }
    if(!found){
        cout<< "不在群内无法聊天" << endl;
        waiting();
        return;
    }

    // 设置当前群聊
    current_chat_group = target_group;

    // // 拉取离线消息
    // json req;
    // req["type"] = "get_unread_group_messages";
    // req["token"] = token;    
    // req["group_name"] = target_group;
    // send_json(sock, req);
    // sem_wait(&sem); // 等待信号量

    cout << "进入 [" << target_group << "] 群" << endl;
    cout << "提示："<< endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file [路径] 传输文件" << endl;
    cout << "- 输入 /exit 退出" << endl;
    cout << "- 输入 /help 获取提示" << endl;
    while (true) {
        // string message = readline_string("> ");
        // if (message.empty()) {
        //     continue;
        // }



        std::string message;   // 用来接收每一行的字符串变量
        std::getline(std::cin,message);

        if (message == "/exit") {
            // 退出当前群聊
            current_chat_group = "";
            cout << "[系统] 已退出群聊模式。" << endl;
            break;
        }

        if (message == "/help") {
            cout << "提示："<< endl;
            cout << "- 输入消息并回车发送" << endl;
            cout << "- 输入 /history [数量] 查看历史记录" << endl;
            cout << "- 输入 /file [路径] 传输文件" << endl;
            cout << "- 输入 /exit 退出" << endl;
            cout << "- 输入/help 获取提示" << endl;
            continue;
        }

        if (message.rfind("/history", 0) == 0) {
            int count = 10;
            std::istringstream iss_history(message);
            string cmd;
            iss_history >> cmd >> count;

            json history;
            history["type"]="get_group_history";
            history["token"]=token;
            history["group_name"]=target_group;
            history["count"]=count;
            send_json(sock, history);
            sem_wait(&sem);  
            continue;
        }

        if (message.rfind("/file", 0) == 0) {
json jjj;
jjj["type"] = "show_group_list";
jjj["token"] = token;
send_json(sock, jjj);
sem_wait(&sem);
bool found1 = false;
for (const auto& list : friend_list) {
    if (list == target_group) {
        found1 = true;
        break;
    }
}
if(!found1){
    cout<< "不在群无法发送文件" << endl;
    waiting();
    return;
}
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                waiting();
                continue;
            }

            // 判断是不是目录
            if (fs::is_directory(path)) {
                cout << "[错误] 不能传输目录！" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                waiting();
                continue;
            }

            // 提取文件名和大小
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) {
                cout << "[错误] 文件打开失败！" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                waiting();
                continue;
            }
            long long filesize_ll = file.tellg();
            std::string filesize = std::to_string(filesize_ll);
            // string filename = path.substr(path.find_last_of("/") + 1);

// 修改filename作为 account+是否群消息(p/g)+filename

            string filename1 = path.substr(path.find_last_of("/") + 1);
            string filename = "g" + unique_account + filename1;



            std::thread([sock, token, target_group , path, filename,filesize,filename1]() {
                const std::string ftp_ip = "10.30.1.215";
                const int ftp_port = 2100;

                int control_fd = connect_to_server(ftp_ip, ftp_port);
                if (control_fd < 0) {
                    std::cerr << "[错误] 连接FTP控制端失败\n" << endl;
                    return;
                }

                ftp_stor(control_fd, filename, path);

                close(control_fd);


                // 文件发送成功告诉服务端推送文件
                json file_req;
                file_req["type"] = "send_group_file";
                file_req["token"] = token;
                file_req["group_name"] = target_group;
                file_req["filename"] = filename1;
                file_req["filesize"] = filesize;
                send_json(sock, file_req);

                // cout << "文件上传完成" << endl;
                        
            }).detach();
            cout << "[系统] 文件上传后台进行中..." << endl;
            continue;
        }

        json msg;
        msg["type"]= "send_group_message";
        msg["token"]=token;
        msg["group_name"]=target_group;
        msg["message"]=message;

        send_json(sock, msg);
        // sem_wait(&sem);  

    }
    // 意外退出，理论上不会到这
    current_chat_group = "";
}

// 处理粘包：从 control_fd 逐行读取服务器响应（以 \r\n 分隔）
std::string read_ftp_response_line(int control_fd) {
    static std::string buffer;
    char temp[512];

    while (true) {
        size_t pos = buffer.find("\r\n");
        if (pos != std::string::npos) {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 2);
            return line;
        }

        ssize_t n = recv(control_fd, temp, sizeof(temp) - 1, 0);
        if (n <= 0) return "";
        temp[n] = '\0';
        buffer += temp;
    }
}

// 创建并连接服务器socket
int connect_to_server(const string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        cerr << "[错误] IP地址格式错误: " << ip << endl;
        close(sock);
        return -1;
    }

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

// 进入PASV模式，返回数据端口
pair<string, int> enter_passive_mode(int control_fd) {
    const char* pasv_cmd = "PASV\r\n";
    if (send(control_fd, pasv_cmd, strlen(pasv_cmd), 0) < 0) {
        perror("send PASV");
        return {"", -1};
    }

    std::string buf = read_ftp_response_line(control_fd);


    // cout << "[调试] 服务器PASV响应: " << buf << endl;

    // 解析类似 "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)"
    int h1,h2,h3,h4,p1,p2;
    if (sscanf(buf.c_str(), "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1,&h2,&h3,&h4,&p1,&p2) != 6) {
        cerr << "[错误] 解析PASV响应失败" << endl;
        return {"", -1};
    }

    string ip = to_string(h1) + "." + to_string(h2) + "." + to_string(h3) + "." + to_string(h4);
    int port = p1 * 256 + p2;

    // cout << "[调试] 解析得到IP: " << ip << ", 端口: " << port << endl;

    return {ip, port};
}



void ftp_stor(int control_fd, const std::string& filename, const std::string& filepath) {
    auto [ip, port] = enter_passive_mode(control_fd);
    if (ip.empty() || port < 0) {
        std::cerr << "[错误] 进入PASV模式失败\n";
        return;
    }

    std::string stor_cmd = "STOR " + filename + "\r\n";
    send(control_fd, stor_cmd.c_str(), stor_cmd.size(), 0);

    // 等待服务器回复150
    std::string resp_150 = read_ftp_response_line(control_fd);
    if (resp_150.find("150") == std::string::npos) {
        std::cerr << "[错误] 未收到150: " << resp_150 << std::endl;
        return;
    }

    int data_fd = connect_to_server(ip, port);
    if (data_fd < 0) {
        std::cerr << "[错误] 连接数据端口失败\n";
        return;
    }

    int file_fd = open(filepath.c_str(), O_RDONLY);
    if (file_fd < 0) {
        std::cerr << "[错误] 打开文件失败: " << filepath << std::endl;
        close(data_fd);
        return;
    }
    
    off_t offset = 0;
    struct stat file_stat;
    fstat(file_fd, &file_stat);
    size_t remaining = file_stat.st_size;

    // while (remaining > 0) {
    //     ssize_t sent = sendfile(data_fd, file_fd, &offset, remaining);
    //     if (sent <= 0) break;
    //     remaining -= sent;
    // }

    while (remaining > 0) {
        ssize_t sent = sendfile(data_fd, file_fd, &offset, remaining);
        if (sent <= 0) {
            perror("sendfile error or done");
            break;
        }
        remaining -= sent;
        // std::cout << "sendfile sent bytes: " << sent << ", remaining: " << remaining << std::endl;
    }

    shutdown(data_fd, SHUT_WR);

    close(file_fd);
    //close(data_fd);

    // 接收服务器确认消息




    std::string resp_226 = read_ftp_response_line(control_fd);
    if (resp_226.find("226") != std::string::npos){
    std::cout << "文件上传完成" << std::endl;
        //std::cout << "[系统] 上传完成: " << filename << std::endl;
    } else {
        std::cout << "[系统] 上传响应: " << resp_226 << std::endl;
    }
    close(data_fd);
}




void ftp_retr(int control_fd, const std::string& filename, const std::string& save_path) {
    // 进入PASV模式，获取数据端口
    auto [ip, port] = enter_passive_mode(control_fd);
    if (ip.empty() || port < 0) {
        std::cerr << "[错误] 进入PASV模式失败\n";
        return;
    }

    // 发送RETR命令
    std::string retr_cmd = "RETR " + filename + "\r\n";
    send(control_fd, retr_cmd.c_str(), retr_cmd.size(), 0);

    std::string resp_150 = read_ftp_response_line(control_fd);
    if (resp_150.find("150") == std::string::npos) {
        std::cerr << "[错误] 未收到150: " << resp_150 << std::endl;
        return;
    }

    // 连接数据端口
    int data_fd = connect_to_server(ip, port);
    if (data_fd < 0) {
        std::cerr << "[错误] 连接数据端口失败\n";
        return;
    }


    // 打开本地文件准备写入
    std::ofstream ofs(save_path, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "[错误] 无法打开本地文件保存路径: " << save_path << std::endl;
        close(data_fd);
        return;
    }

    // 从数据连接读取数据，写入文件
    char buf[4096];
    ssize_t m;
    while ((m = recv(data_fd, buf, sizeof(buf), 0)) > 0) {
        ofs.write(buf, m);
    }
    ofs.close();
    close(data_fd);

        std::string resp_226 = read_ftp_response_line(control_fd);
    if (resp_226.find("226") != std::string::npos){
        std::cout << "文件下载完成" << std::endl;
        //std::cout << "[系统] 下载完成: " << filename << std::endl;
    } else {
        std::cout << "[系统] 下载响应: " << resp_226 << std::endl;
    }
}