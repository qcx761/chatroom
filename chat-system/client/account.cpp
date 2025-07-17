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

void destory_account(MenuState& state,std::atomic<bool>& login_success){
    system("clear");
    cout <<"注销函数 :" << endl;
    cout<<"确定要注销函数吗(Y/N) :";
    char a;
    cin>>a;
    if(a=='Y'){

        
// 注销函数实现







        login_success.store(false);
        state=main_menu;
        cout<<"成功注销"<<endl;
    }else if(a=='N'){
        return;
    }else{
        cout<< "未知输入，默认取消注销"<< endl;
    }

    // 不会跑到这
    flushInput();
    waiting();
}

void quit_account(MenuState& state,std::atomic<bool>& login_success){

// 处理在线状态之类的，退出函数


    login_success.store(false);
    state=main_menu;
}
void username_view(){
    ;
}
void username_change(){
    ;
}
void password_change(){
    ;
}
