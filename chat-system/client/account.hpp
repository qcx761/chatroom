#include <iostream>
#include <semaphore.h>
#include <atomic>
#include <termios.h>// 隐藏终端输入
#include <unistd.h>
#include <nlohmann/json.hpp>
#include"menu.hpp"
using namespace std;

using json = nlohmann::json;




void flushInput();
void waiting();
void main_menu_ui(int sock,sem_t& sem,std::atomic<bool>& login_success);
// void main_menu_ui(int sock);
// void log_in(int sock);
void log_in(int sock,sem_t& sem);
// void sign_up(int sock);
void sign_up(int sock,sem_t& sem);
string get_password(const string& prompt);








// void send_json(int sock,json j);
