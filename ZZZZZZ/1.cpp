void log_in_UI(int connecting_sockfd) {
    system("clear");
    std::cout << "登录" << std::endl;

    json j;
    j["type"] = "log_in";
    std::string username, password;

    std::cout << "请输入你的用户名：";
    std::cin >> username;
    j["username"] = username;

    struct termios oldt, newt;

    // 获取当前终端设置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 关闭回显
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << "请输入你的密码：";
    std::cin >> password;
    j["password"] = password;

    std::cout << std::endl;

    // 恢复终端设置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    send_json(connecting_sockfd, j);
    sem_wait(&semaphore); // 等待信号量
}

void sign_up_UI(int connecting_sockfd) {
    system("clear");
    std::cout << "注册" << std::endl;

    json j;
    j["type"] = "sign_up";
    std::string username, password, security_question, security_answer;

    std::cout << "请输入你的用户名：";
    std::cin >> username;
    j["username"] = username;

    std::cout << "请输入你的密码：";
    std::cin >> password;
    j["password"] = password;

    std::cout << "请输入你的密保问题：";
    std::cin.ignore();
    std::getline(std::cin, security_question);
    j["security_question"] = security_question;

    std::cout << "请输入你的密保答案：";
    std::getline(std::cin, security_answer);
    j["security_answer"] = security_answer;

    send_json(connecting_sockfd, j);
    sem_wait(&semaphore); // 等待信号量
}



























#include"account.hpp"

sem_t sem; // 定义信号量



void waiting() {
    std::cout << "按 Enter 键继续...";
    std::cin.get();  // 等待按回车
}

void flushInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void main_menu_ui(int sock) {





sem_init(&sem, 0, 0);




    int n;
    while (1) {
        system("clear"); // 清屏
        show_main_menu();

        std::cout << "请输入你的选项：";
        if (!(std::cin >> n)) {
            flushInput();
            std::cout << "无效的输入，请输入数字。" << std::endl;
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
            std::cout << "无效数字" << std::endl;
            flushInput();
            waiting();
            break;
        }
    }
}

void log_in(int sock){
    system("clear");
    std::cout << "登录" << std::endl;
    json j;
    j["type"]="log_in";

    string username,password;
    cout<<"请输入用户名 :";

    cin >> username;
    cout<<"请输入密码   :";

    cin >> password;
    j["username"]=username;
    j["password"]=password;
    send_json(sock, j);

    sem_wait(&sem); // 等待信号量




}

void sign_up(int sock){
    system("clear");
    std::cout << "注册" << std::endl;
    json j;
    j["type"]="sign_up";
    string username,password_old,password_new;
    while(1){
    cout<<"请输入用户名 :";
    cin >> username;

    cout<<"请输入密码   :";
    cin >> password_old;
    cout<<"请再次输入密码:";
    cin >> password_new;
    if(password_new==password_old){
        break;
    }else{
    cout<< "两次密码不一样"<<endl;
    }
}
    j["username"]=username;
    j["password"]=password_old;
    send_json(sock, j);

    sem_wait(&sem); // 等待信号量
}

void send_json(int sock,json j){
    ;
}
