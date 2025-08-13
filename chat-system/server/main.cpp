#include"server.hpp"

int main(int argc, char* argv[]) {
    
    std::string port_str(argv[1]);
    int port = std::stoi(port_str);

    Server Server(port);

    return 0;
}




// #include"server.hpp"

// #include <gperftools/profiler.h>
// #include <csignal>
// #include <cstdlib>

// void signalHandler(int signum) {
//     ProfilerStop();  // 停止采样
//     std::exit(signum);
// }

// int main(int argc, char* argv[]) {

//     ProfilerStart("cpu.prof");  // 开始采样，文件名自定义

//     std::string port_str(argv[1]);
//     int port = std::stoi(port_str);
//     std::signal(SIGINT, signalHandler);
//     Server Server(port);


//     return 0;
// }