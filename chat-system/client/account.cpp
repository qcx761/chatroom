#include "account.hpp"
#include "json.hpp"
#include "msg.hpp"


void waiting() {
    cout << "按 Enter 键继续...";
    cin.get();  // 等待按回车
}

void flushInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

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

        // cout << "请输入你的选项：";
        if (!(cin >> n)) {
            flushInput();
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
            flushInput();
            waiting();
            break;
        }
    }
        // system("clear"); // 清屏
}

void log_in(int sock,sem_t& sem) {
    system("clear");
    cout << "登录" << endl;
    json j;
    j["type"] = "log_in";

    string account, password;
    cout << "请输入帐号   :";
    cin >> account;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 清理缓冲区换行符

    password = get_password("请输入密码   :");

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
    string username,account, password_old, password_new;

        cout << "请输入用户名 :";
        cin >> username;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

        cout << "请输入帐号  :";
        cin >> account;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 防止影响后续 getline

        password_old = get_password("请输入密码   :");
        password_new = get_password("请再次输入密码:");

        if (password_new == password_old) {
            j["account"]  = account;
            j["username"] = username;
            j["password"] = password_old;

            send_json(sock, j);

            sem_wait(&sem); // 等待信号量
        } else {
            cout << "两次密码不一样" << endl;
            return;
        }
}

void destory_account(int sock,string token,sem_t& sem){
    system("clear");
    cout <<"注销函数 :" << endl;
    cout<<"确定要注销帐号吗(Y/N) :";
    char a;
    cin>>a;
    if(a=='Y'){
        string account,password;
        cout << "请输入帐号  :";
        cin >> account;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        password = get_password("请输入密码   :");
        json j;
        j["type"] = "destory_account";
        j["token"] = token;
        j["account"]=account;
        j["password"]=password;
        send_json(sock, j);
    }else if(a=='N'){
        return;
    }else{
        cout<< "未知输入，取消注销"<< endl;
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
    flushInput();
    waiting();
}

void username_view(int sock,string token,sem_t& sem){
    json j;
    j["type"]="username_view";
    j["token"]=token;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    flushInput();
    waiting();
}

void username_change(int sock,string token,sem_t& sem){
    system("clear");
    cout <<"修改用户名 :" << endl;
    string username;
    cout << "请输入想修改的用户名  :";
    cin >> username;
    json j;
    j["type"]="username_change";
    j["token"]=token;
    j["username"]=username;
    send_json(sock,j);
    sem_wait(&sem); // 等待信号量
    flushInput();
    waiting();
}

void password_change(int sock,string token,sem_t& sem){
    system("clear");
    cout <<"密码修改 :" << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string password_new,password_old,password_new1;

    password_old = get_password("请输入旧密码   :");
    password_new = get_password("请输入新密码   :");
    password_new1 = get_password("请再次输入新密码:");

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
    cout <<"好友列表 :" << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
    cout <<"添加用户 :" << endl;
    string target_username;
    cout << "请输入想添加的用户名  :";
    cin >> target_username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

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
    cout <<"删除用户 :" << endl;
    string username;
    cout << "请输入想删除的用户名  :";
    cin >> username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

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
    cout <<"解除屏蔽用户 :" << endl;
    string target_username;
    cout << "请输入想解除屏蔽的用户名  :";
    cin >> target_username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

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
    cout <<"屏蔽用户 :" << endl;
    string target_username;
    cout << "请输入想屏蔽的用户名  :";
    cin >> target_username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

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
        flushInput();
        waiting();
        return;
    }

    std::cout << "输入处理编号(0 退出): ";
    int choice;
    std::cin >> choice;
    flushInput();
    char op;
    std::string from_username;
{
    std::lock_guard<std::mutex> lock(friend_requests_mutex);

    if (choice <= 0 || choice > global_friend_requests.size()) {
        std::cout << "已取消处理。" << std::endl;
        waiting();
        return;
    }

    from_username = global_friend_requests[choice - 1]["username"];

    std::cout << "你想如何处理 [" << from_username << "] 的请求？(a=接受, r=拒绝): ";
    std::cin >> op;
    flushInput();
}
    std::string action;
    if (op == 'a' || op == 'A') {
        action = "accept";
    } else if (op == 'r' || op == 'R') {
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
    cout <<"好友信息查询 :" << endl;
    string target_username;
    cout <<"输入查找的好友名 :";
    cin >> target_username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
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

    string target_username;
    cout << "请输入想私聊的用户名(必须是好友否则无法发送成功) : ";
    cin >> target_username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');





// 整个函数来实现并判断是否时好友
// 设置 current_chat_target = 这个好友的账号，
// 退出私聊要设置为空数组，我在msg里面定义就好在这个函数里面改变实现
// 拉历史消息
// 把之前未读的离线消息标记为已读（可选）
// 拉取历史消息实现,改为已读
// 函数实现 get_unread_private_message


    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录" << endl;

    string message;
    while (true) {
        cout << "> ";
        getline(cin, message);

        if (message == "/exit") {
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
        }

        if (message.rfind("/history", 0) == 0) {
            int count = 10;
            std::istringstream iss(message);
            string cmd;
            iss >> cmd >> count;

            json history;
            history["type"]="get_private_history";
            history["token"]=token;
            history["target_username"]=target_username;
            history["count"]=count;
            send_json(sock, history);
            sem_wait(&sem);  
            // 要区分接收者是在线还是离线状态   

            // 等待服务端响应，offline_pri 会被填充


// 在线状态直接实现好友信息时时输出，维护一个变量来知道目前在私聊界面与否

            };

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