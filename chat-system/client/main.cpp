#include<iostream>
#include <csignal>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include"client.hpp"

std::atomic<bool> g_running(true);
std::condition_variable cv;
std::mutex cv_mtx;

void handle_sigint(int sig) {
    std::cout << "\nCtrl+C detected, stopping client..." << std::endl;
    g_running = false;
    cv.notify_one();
}

int main(int argc, char* argv[]) {

    signal(SIGINT, handle_sigint);

    std::string ip = argv[1];
    std::string port_str(argv[2]);
    int port = std::stoi(port_str);
    Client client(ip, port);

        // 阻塞等待 Ctrl+C
    {
        std::unique_lock<std::mutex> lock(cv_mtx);
        cv.wait(lock, [] { return !g_running.load(); });
    }

    return 0;
}

