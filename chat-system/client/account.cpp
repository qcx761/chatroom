#include "account.hpp"
#include "json.hpp"



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


void handle_friend_request_msg(int sock,string token,sem_t& sem){
    system("clear");

    json j;
    j["type"]="handle_friend_request_msg";
    j["token"]=token;






    send_json(sock,j);
    sem_wait(&sem);
    // flushInput();
    waiting();
}



void show_friend_msg(int sock,string token,sem_t& sem){
    system("clear");

    json j;
    j["type"]="show_friend_msg";
    j["token"]=token;





        send_json(sock,j);
    sem_wait(&sem);
    // flushInput();
    waiting();
}
// 清屏，token，state，login——success，return返回，发送send，信号量