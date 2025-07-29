#include <iostream>
#include <semaphore.h>
#include <atomic>
#include <termios.h>// 隐藏终端输入
#include <unistd.h>
#include <nlohmann/json.hpp>



#include <readline/readline.h>   // 主要函数 readline()
#include <readline/history.h>    // 可选：支持历史记录





#include"menu.hpp"
using namespace std;

using json = nlohmann::json;

string readline_string(const string& prompt);

// void flushInput();
void waiting();
void log_in(int sock,sem_t& sem);
void sign_up(int sock,sem_t& sem);
string get_password(const string& prompt);
void main_menu_ui(int sock,sem_t& sem,std::atomic<bool>& login_success);

void destory_account(int sock,string token,sem_t& sem);
void quit_account(int sock,string token,sem_t& sem);
void username_view(int sock,string token,sem_t& sem);
void username_change(int sock,string token,sem_t& sem);
void password_change(int sock,string token,sem_t& sem);
void show_friend_list(int sock,string token,sem_t& sem);
void add_friend(int sock,string token,sem_t& sem);
void remove_friend(int sock,string token,sem_t& sem);
void unmute_friend(int sock,string token,sem_t& sem);
void mute_friend(int sock,string token,sem_t& sem);
void getandhandle_friend_request(int sock,string token,sem_t& sem);
void get_friend_info(int sock,const string& token,sem_t& sem);




void send_private_message(int sock, const string& token, sem_t& sem);