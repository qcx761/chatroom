#include <iostream>
#include <semaphore.h>
#include <atomic>
#include <termios.h>// 隐藏终端输入
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <thread>




#include <fcntl.h>      // open, O_RDONLY
#include <sys/stat.h>   // struct stat, stat, fstat
#include <sys/sendfile.h>  // sendfile
#include <unistd.h>     // close, read, write 等系统调用
#include <fstream>




#include <readline/readline.h>   // 主要函数 readline()
#include <readline/history.h>    // 可选：支持历史记录





#include"menu.hpp"
using namespace std;

using json = nlohmann::json;

string readline_string(const string& prompt);
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
void get_friend_info(int sock,string token,sem_t& sem);
void send_private_message(int sock,string token, sem_t& sem);
void show_group_list(int sock,string token,sem_t& sem);
void join_group(int sock,string token,sem_t& sem);
void quit_group(int sock,string token,sem_t& sem);
void show_group_members(int sock,string token,sem_t& sem);
void create_group(int sock,string token,sem_t& sem);
void set_group_admin(int sock,string token,sem_t& sem);
void remove_group_admin(int sock,string token,sem_t& sem);
void remove_group_member(int sock,string token,sem_t& sem);
void add_group_member(int sock,string token,sem_t& sem);
void dismiss_group(int sock,string token,sem_t& sem);
void get_group_request(int sock,string token,sem_t& sem);
void handle_group_request(int sock,string token,sem_t& sem);
void get_unread_group_messages(int sock,string token,sem_t& sem);
void receive_group_messages(int sock,string token,sem_t& sem);
void get_group_history(int sock,string token,sem_t& sem);
void send_group_message(int sock,string token,sem_t& sem);
void getandhandle_group_request(int sock,string token,sem_t& sem);
void receive_file(int sock,string token,sem_t& sem);
std::string read_ftp_response_line(int control_fd);
int connect_to_server(const string& ip, int port) ;
pair<string, int> enter_passive_mode(int control_fd) ;
void ftp_stor(int control_fd, const std::string& filename, const std::string& filepath) ;
void ftp_retr(int control_fd, const std::string& filename, const std::string& save_path) ;


