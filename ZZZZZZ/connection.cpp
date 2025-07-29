/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
string readline_string(const string& prompt) {
    char* input = readline(prompt.c_str());
    if (!input) return "";
    string result(input);
    if (!result.empty()) add_history(i/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量/ account.cpp的没有
#include "account.hp
#include "json.hpp"
#include "msg.hpp
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量nput);
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
    int choice = stoi(input);

    // flushInput();
// 要不要取消锁
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
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

void get_friend_info(int sock,const string& token,sem_t& sem){
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

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    // 设置当前私聊用户
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem); // 等待信号量

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "提示："<< endl;
    // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
    cout << "- 输入消息并回车发送" << endl;
    cout << "- 输入 /history [数量] 查看历史记录" << endl;
    cout << "- 输入 /file 进入文件传输模式" << endl;
    cout << "- 输入 /exit 退出" << endl;
    while (true) {
        string message = readline_string("> ");
        if (message == "/exit") {
            // 退出当前私聊用户
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
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
            string path;
            std::istringstream iss_file(message);
            string cmd;
            iss_file >> cmd >> path;

            if(path.empty()){
                cout << "未输入路径" << endl;
                cout << "[系统] 已退出文件传输模式。" << endl;
                break;
            }

            json file;

            // file["type"]="send_file";
            // file["token"]=token;
            // file["target_username"]=target_username;
            // file["path"]=path;
            // send_json(sock, file);
            // sem_wait(&
            continue;
        }

        json msg;
        msg["type"]= "send_private_message";
        msg["token"]=token;
        msg["target_username"]=target_username;
        msg["message"]=message;

        send_json(sock, msg);
        sem_wait(&sem);  

    }
}

// 清屏，token，state，login——success，return返回，发送send，信号量