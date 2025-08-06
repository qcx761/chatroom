#include "ftpserver.hpp"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <sys/sendfile.h>
#include <ctime>
#include <cerrno>

using namespace std;

ControlConnect::ControlConnect(int fd, int cmd, const std::string& fname) :
    control_fd(fd), command_type(cmd), filename(fname) {}

void ConnectionGroup::unbind_control_from_data(int data_fd) {
    lock_guard<mutex> lock(mtx);
    data_to_control.erase(data_fd);
}

void ConnectionGroup::bind_data_to_control(int data_fd, int control_fd) {
    lock_guard<mutex> lock(mtx);
    data_to_control[data_fd] = control_fd;
}

int ConnectionGroup::get_control_from_data(int data_fd) {
    lock_guard<mutex> lock(mtx);
    auto it = data_to_control.find(data_fd);
    return (it == data_to_control.end()) ? -1 : it->second;
}

void ConnectionGroup::add_or_update(int fd, int cmd, const std::string& fname) {
    lock_guard<mutex> lock(mtx);
    auto it = find_if(connections.begin(), connections.end(),
        [fd](const ControlConnect& c) { return c.control_fd == fd; });
    if (it == connections.end()) {
        connections.emplace_back(fd, cmd, fname);
    } else {
        it->command_type = cmd;
        if (!fname.empty()) it->filename = fname;
    }
}

void ConnectionGroup::remove(int fd) {
    lock_guard<mutex> lock(mtx);
    connections.erase(remove_if(connections.begin(), connections.end(),
        [fd](const ControlConnect& c) { return c.control_fd == fd; }), connections.end());
    data_to_control.erase(fd);
}

int ConnectionGroup::get_command_type(int fd) {
    lock_guard<mutex> lock(mtx);
    auto it = find_if(connections.begin(), connections.end(),
        [fd](const ControlConnect& c) { return c.control_fd == fd; });
    return (it == connections.end()) ? 0 : it->command_type;
}

std::string ConnectionGroup::get_filename(int fd) {
    lock_guard<mutex> lock(mtx);
    auto it = find_if(connections.begin(), connections.end(),
        [fd](const ControlConnect& c) { return c.control_fd == fd; });
    return (it == connections.end()) ? "" : it->filename;
}

void ConnectionGroup::add_listen_socket(int listen_fd, int control_fd) {
    lock_guard<mutex> lock(mtx);
    listen_to_control[listen_fd] = control_fd;
}

int ConnectionGroup::get_control_from_listen(int listen_fd) {
    lock_guard<mutex> lock(mtx);
    auto it = listen_to_control.find(listen_fd);
    return (it == listen_to_control.end()) ? -1 : it->second;
}

void ConnectionGroup::remove_listen_socket(int listen_fd) {
    lock_guard<mutex> lock(mtx);
    listen_to_control.erase(listen_fd);
}

int ConnectionGroup::get_listen_from_control(int control_fd) {
    lock_guard<mutex> lock(mtx);
    for (const auto& pair : listen_to_control) {
        if (pair.second == control_fd) return pair.first;
    }
    return -1;
}

FTPServer::FTPServer(int port) :
    control_port(port), epfd(-1), server_fd(-1), is_running(false) {
    srand(time(nullptr));
}

bool FTPServer::init() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return false;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(control_port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        return false;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        return false;
    }
    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1 failed");
        close(server_fd);
        return false;
    }
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl ADD server_fd failed");
        close(server_fd);
        close(epfd);
        return false;
    }
    is_running = true;
    return true;
}

void FTPServer::run() {
    epoll_event events[1024];
    while (is_running) {
        int n = epoll_wait(epfd, events, 1024, -1);
        if (n == -1) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            uint32_t evs = events[i].events;
            // if ((evs & EPOLLERR) || (evs & EPOLLHUP) || (evs & EPOLLRDHUP)) {
            //     close_connection(fd);
            //     cout << evs <<endl;
            //     continue;
            // }
            if ((evs & EPOLLERR) || (evs & EPOLLHUP)) {
                close_connection(fd);
                cout << evs <<endl;
                continue;
            }
            if (fd == server_fd) {
                accept_new_control_connection();// 接受新的控制连接，控制连接监听fd
            } else {
                int port = get_socket_local_port(fd);
                if (port == control_port) { 
                    handle_control_fd(fd);// 处理控制命令，控制连接fd
                } else {
                    int control_fd = group.get_control_from_data(fd); //判断该 fd 是否是数据连接
                    if (control_fd == -1) {
                        control_fd = group.get_control_from_listen(fd);
                        if (control_fd != -1) { // 监听socket处理数据连接，数据连接监听fd
                            accept_new_data_connection(fd, control_fd);
                        } else {
                            cerr << "No control connection associated with fd: " << fd << endl;
                            close_connection(fd);
                        }
                    } else { // 数据连接
                        if (evs & EPOLLOUT) {
                            auto it = send_states.find(fd);
                            if (it != send_states.end()) {
                                sendfile_continue(it->second);
                            }
                        }
                        if (evs & EPOLLIN) {
                            int cmd = group.get_command_type(control_fd);
                            if (cmd == 2) { // STOR数据接收
                                handle_stor_data(fd, control_fd);
                            }
                        }
                    }
                }
            }
        }
    }
}

void FTPServer::stop() {
    is_running = false;
    if (server_fd != -1) close(server_fd);
    if (epfd != -1) close(epfd);
}

void FTPServer::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int FTPServer::get_socket_local_port(int fd) {
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (getsockname(fd, (sockaddr*)&addr, &len) == -1) {
        perror("getsockname");
        return -1;
    }
    return ntohs(addr.sin_port);
}

void FTPServer::close_connection(int fd) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    group.remove(fd);

    int listen_fd = group.get_listen_from_control(fd);
    if (listen_fd != -1) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, listen_fd, nullptr);
        close(listen_fd);
        group.remove_listen_socket(listen_fd);
    }

    send_states.erase(fd);
    stor_states.erase(fd);

    cout << "Closed fd: " << fd << endl;
}

void FTPServer::accept_new_control_connection() {
    sockaddr_in cli_addr{};
    socklen_t cli_len = sizeof(cli_addr);
    int new_fd = accept(server_fd, (sockaddr*)&cli_addr, &cli_len);
    if (new_fd < 0) {
        perror("accept");
        return;
    }
    set_nonblocking(new_fd);
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    ev.data.fd = new_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &ev) == -1) {
        perror("epoll_ctl add new control_fd");
        close(new_fd);
        return;
    }
    group.add_or_update(new_fd, 0);
    cout << "Accepted new control connection fd: " << new_fd << endl;
}

void FTPServer::handle_control_fd(int fd) {
    char buf[1024];
    int n = recv(fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close_connection(fd);
        return;
    }
    buf[n] = '\0';
    string cmd(buf);
    cout << "Received command from fd " << fd << ": " << cmd << endl;

    if (cmd.find("PASV") != string::npos) {
        setup_passive_listen(fd);
    }
    else if (cmd.find("STOR ") == 0) {
        string filename = cmd.substr(5);
        filename.erase(remove(filename.begin(), filename.end(), '\n'), filename.end());
        filename.erase(remove(filename.begin(), filename.end(), '\r'), filename.end());
        group.add_or_update(fd, 2, filename);
        cout << "Prepare to receive file: " << filename << endl;

    std::string msg = "150 Opening data connection.\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    cout << "send 150" <<endl;
    }
    else if (cmd.find("RETR ") == 0) {
        string filename = cmd.substr(5);
        filename.erase(remove(filename.begin(), filename.end(), '\n'), filename.end());
        filename.erase(remove(filename.begin(), filename.end(), '\r'), filename.end());
        group.add_or_update(fd, 1, filename);
        cout << "Prepare to send file: " << filename << endl;

    std::string msg = "150 Opening data connection.\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    cout << "send 150" <<endl;

    }
    else {
        // 其他命令暂不处理 
    }



}

void FTPServer::setup_passive_listen(int control_fd) {
    int pasv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (pasv_fd < 0) {
        perror("socket pasv_fd");
        return;
    }
    int opt = 1;
    setsockopt(pasv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = 0;

    if (bind(pasv_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind pasv_fd");
        close(pasv_fd);
        return;
    }
    if (listen(pasv_fd, 10) < 0) {
        perror("listen pasv_fd");
        close(pasv_fd);
        return;
    }
    sockaddr_in sin{};
    socklen_t len = sizeof(sin);
    if (getsockname(pasv_fd, (sockaddr*)&sin, &len) == -1) {
        perror("getsockname pasv_fd");
        close(pasv_fd);
        return;
    }
    int port = ntohs(sin.sin_port);

    set_nonblocking(pasv_fd);
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = pasv_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, pasv_fd, &ev) == -1) {
        perror("epoll_ctl add pasv_fd");
        close(pasv_fd);
        return;
    }

    group.add_listen_socket(pasv_fd, control_fd);

    // 发送PASV响应给客户端
    const char* ipstr = "10,30,1,215";
    int p1 = port / 256;
    int p2 = port % 256;

    char response[100];
    snprintf(response, sizeof(response),
             "227 Entering Passive Mode (%s,%d,%d)\r\n", ipstr, p1, p2);
    send(control_fd, response, strlen(response), 0);

    cout << "PASV mode on port: " << port << endl;
}

void FTPServer::accept_new_data_connection(int listen_fd, int control_fd) {
    sockaddr_in cli_addr{};
    socklen_t cli_len = sizeof(cli_addr);
    int data_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);
    if (data_fd < 0) {
        perror("accept data connection");
        return;
    }
cout <<"数据连接 fd="<< data_fd <<endl;
    set_nonblocking(data_fd);
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLOUT;
    ev.data.fd = data_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, data_fd, &ev) == -1) {
        perror("epoll_ctl add data_fd");
        close(data_fd);
        return;
    }
    group.bind_data_to_control(data_fd, control_fd);

    int cmd = group.get_command_type(control_fd);
    string filename = group.get_filename(control_fd);

    if (cmd == 1) { // RETR
        start_sendfile(data_fd, control_fd, filename);
    } else if (cmd == 2) { // STOR
        start_stor(data_fd, control_fd, filename);
    }




}

void FTPServer::start_sendfile(int data_fd, int control_fd, const string& filename) {
    int file_fd = open(filename.c_str(), O_RDONLY);
    if (file_fd < 0) {
        perror("open file for RETR");
        close_connection(data_fd);
        return;
    }

    struct stat st{};
    if (fstat(file_fd, &st) < 0) {
        perror("fstat file for RETR");
        close(file_fd);
        close_connection(data_fd);
        return;
    }
    SendState state{};
    state.file_fd = file_fd;
    state.offset = 0;
    state.filesize = st.st_size;
    state.data_fd = data_fd;
    state.control_fd = control_fd;
    state.filename = filename;
    state.active = true;
    send_states[data_fd] = state;

    // 立即尝试发送一次
    sendfile_continue(state);
}

void FTPServer::sendfile_continue(SendState& state) {
    while (state.offset < state.filesize) {
        ssize_t n = sendfile(state.data_fd, state.file_fd, &state.offset, state.filesize - state.offset);
        if (n == 0) break; // EOF
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 发送缓冲区满，等待下次EPOLLOUT触发继续发送
                return;
            } else {
                perror("sendfile");
                close_connection(state.data_fd);
                return;
            }
        }
    }
    // 文件发送完毕
    close(state.file_fd);
    send_states.erase(state.data_fd);

    // 关闭数据连接
    close_connection(state.data_fd);

    // 给控制连接发传输完成消息
    const char* msg = "226 Transfer complete.\r\n";
    send(state.control_fd, msg, strlen(msg), 0);
}

void FTPServer::start_stor(int data_fd, int control_fd, const string& filename) {
    int file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("open file for STOR");
        close_connection(data_fd);
        return;
    }

    StorState state{};
    state.file_fd = file_fd;
    state.active = true;
    stor_states[data_fd] = state;
}



void FTPServer::handle_stor_data(int data_fd, int control_fd) {
    auto it = stor_states.find(data_fd);
    if (it == stor_states.end()) {
        cerr << "No stor state for data_fd: " << data_fd << endl;
        close_connection(data_fd);
        return;
    }

    char buf[4096];
    while (true) {
        ssize_t n = recv(data_fd, buf, sizeof(buf), 0);

        if (n > 0) {
            // 正常接收到数据，写入文件
            std::cout << "[服务端] recv() 接收到: " << n << " 字节" << std::endl;
            ssize_t written = write(it->second.file_fd, buf, n);
            if (written != n) {
                perror("write error");
                close(it->second.file_fd);
                stor_states.erase(it);
                close_connection(data_fd);
                return;
            }
        } else if (n == 0) {
            // 客户端关闭了数据连接（传输完成）
            std::cout << "[服务端] 客户端关闭数据连接，发送 226" << std::endl;

            close(it->second.file_fd);
            stor_states.erase(it);
            close_connection(data_fd);

            const char* msg = "226 Transfer complete.\r\n";
            send(control_fd, msg, strlen(msg), 0);
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有更多数据了，等下一次 epoll 再进来
                break;
            } else {
                // 发生错误
                perror("recv error");
                close(it->second.file_fd);
                stor_states.erase(it);
                close_connection(data_fd);
                return;
            }
        }
    }
}
