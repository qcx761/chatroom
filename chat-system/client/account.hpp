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
void log_in(int sock,sem_t& sem);
void sign_up(int sock,sem_t& sem);
string get_password(const string& prompt);
void main_menu_ui(int sock,sem_t& sem,std::atomic<bool>& login_success);

void destory_account(int sock,string token,sem_t& sem);
void quit_account(int sock,string token,sem_t& sem);
void username_view(int sock,string token,sem_t& sem);
void username_change(int sock,string token,sem_t& sem);
void password_change(int sock,string token,sem_t& sem);






// void show_friend_list();
// void add_friend();
// void delete_friend();
// void private_chat_friend();
// void block_friend();
// void unblock_friend();
// void show_group_list();
// void create_group();
// void join_group();
// void exit_group();
// void show_group_members();
// void send_group_file();
// void set_group_admin();
// void remove_group_admin();
// void remove_group_member();
// void add_group_member();
// void dismiss_group();
// void show_friend_requests();
// void show_private_messages();
// void show_group_requests();


void show_friend_list(int sock,string token,sem_t& sem);
void add_friend(int sock,string token,sem_t& sem);
void remove_friend(int sock,string token,sem_t& sem);
void unmute_friend(int sock,string token,sem_t& sem);
void mute_friend(int sock,string token,sem_t& sem);


void getandhandle_friend_request(int sock,string token,sem_t& sem);
void show_friend_notifications(int sock,string token,sem_t& sem);
