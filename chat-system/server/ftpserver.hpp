#pragma once


#include <string>
#include <vector>
#include <map>
#include <mutex>

struct ControlConnect {
    int control_fd;
    int command_type; // 0无命令，1 RETR，2 STOR
    std::string filename;
    ControlConnect(int fd, int cmd = 0, const std::string& fname = "");
};

struct SendState {
    int file_fd;
    off_t offset;
    off_t filesize;
    int data_fd;
    int control_fd;
    std::string filename;
    bool active;
};

struct StorState {
    int file_fd;
    bool active;
};

class ConnectionGroup {
public:
    void unbind_control_from_data(int data_fd);
    void bind_data_to_control(int data_fd, int control_fd);
    int get_control_from_data(int data_fd);
    void add_or_update(int fd, int cmd, const std::string& fname = "");
    void remove(int fd);
    int get_command_type(int fd);
    std::string get_filename(int fd);
    void add_listen_socket(int listen_fd, int control_fd);
    int get_control_from_listen(int listen_fd);
    void remove_listen_socket(int listen_fd);
    int get_listen_from_control(int control_fd);

private:
    std::mutex mtx;
    std::vector<ControlConnect> connections;
    std::map<int, int> data_to_control;
    std::map<int, int> listen_to_control;
};

class FTPServer {
public:
    FTPServer(int port);
    bool init();
    void run();
    void stop();

private:
    int control_port;
    int epfd;
    int server_fd;
    bool is_running;
    ConnectionGroup group;
    std::map<int, SendState> send_states;
    std::map<int, StorState> stor_states;
    void set_nonblocking(int fd);
    int get_socket_local_port(int fd);
    void close_connection(int fd);
    void accept_new_control_connection();
    void handle_control_fd(int fd);
    void setup_passive_listen(int control_fd);
    void accept_new_data_connection(int listen_fd, int control_fd);
    void start_sendfile(int data_fd, int control_fd, const std::string& filename);
    void sendfile_continue(SendState& state);
    void start_stor(int data_fd, int control_fd, const std::string& filename);
    void handle_stor_data(int data_fd, int control_fd);
};

