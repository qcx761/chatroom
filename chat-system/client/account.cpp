#include "account.hpp"


sem_t sem; // 定义信号量

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

void main_menu_ui(int sock) {





    sem_init(&sem, 0, 0);  // 初始化信号量










    int n;
    while (1) {
        system("clear"); // 清屏
        show_main_menu();

        cout << "请输入你的选项：";
        if (!(cin >> n)) {
            flushInput();
            cout << "无效的输入，请输入数字。" << endl;
            waiting();
            continue;
        }

        switch (n) {
        case 1:
            log_in(sock);
            flushInput();
            waiting();
            break;
        case 2:
            sign_up(sock);
            flushInput();
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
}

void log_in(int sock) {
    system("clear");
    cout << "登录" << endl;
    json j;
    j["type"] = "log_in";

    string username, password;
    cout << "请输入用户名 :";
    cin >> username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 清理缓冲区换行符

    password = get_password("请输入密码   :");

    j["username"] = username;
    j["password"] = password;
    send_json(sock, j);

    sem_wait(&sem); // 等待信号量
}

void sign_up(int sock) {
    system("clear");
    cout << "注册" << endl;
    json j;
    j["type"] = "sign_up";
    string username, password_old, password_new;

    while (1) {
        cout << "请输入用户名 :";
        cin >> username;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 这里也加，防止影响后续 getline

        password_old = get_password("请输入密码   :");
        password_new = get_password("请再次输入密码:");

        if (password_new == password_old) {
            break;
        } else {
            cout << "两次密码不一样" << endl;
            waiting();
            system("clear");
            cout << "注册" << endl;
        }
    }

    j["username"] = username;
    j["password"] = password_old;
    send_json(sock, j);

    sem_wait(&sem); // 等待信号量
}

void send_json(int sock, json j) {








}
