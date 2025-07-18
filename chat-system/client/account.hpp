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





// void add_friend(const std::string& friend_account);
// void delete_friend(const std::string& friend_account);
// void query_friends();
// void send_private_message(const std::string& friend_account, const std::string& message);
// void block_friend(const std::string& friend_account, bool block);
// void create_group(const std::string& group_name);
// void join_group(const std::string& group_id);
// void leave_group(const std::string& group_id);
// void query_group_members(const std::string& group_id);
// void send_group_message(const std::string& group_id, const std::string& message);
// void fetch_history(const std::string& chat_id, int offset, int limit);

