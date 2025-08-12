#include<iostream>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include"client.hpp"

int main(int argc, char* argv[]) {

    // signal(SIGINT, SIG_IGN);   // 忽略 Ctrl+C (中断信号)
    // signal(SIGTERM, SIG_IGN);  // 忽略 kill 发送的终止信号
    // signal(SIGTSTP, SIG_IGN);  // 忽略 Ctrl+Z (挂起/停止信号)

    // // 忽略 Ctrl+D
    // struct termios tty;
    // if (tcgetattr(STDIN_FILENO, &tty) == 0) {
    //     tty.c_cc[VEOF] = 0;
    //     tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    // }

    std::string ip = argv[1];
    std::string port_str(argv[2]);
    int port = std::stoi(port_str);
    Client client(ip, port);
    client.start();
    // client.stop();  
    return 0;
}


