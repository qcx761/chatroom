#include <iostream>
#include <semaphore.h>


#include <termios.h>// 隐藏终端输入

#include <unistd.h>

#include <nlohmann/json.hpp>

#include"menu.hpp"
using namespace std;

using json = nlohmann::json;

void flushInput();
void waiting();
void main_menu_ui(int sock);
void log_in(int sock);
void sign_up(int sock);
string get_password(const string& prompt);








void send_json(int sock,json j);
