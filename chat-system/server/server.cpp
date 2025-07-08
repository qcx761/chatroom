#include"server.hpp"
#include"../log/logger.hpp"

Server::Server(int Port):port(Port)
{
    listen_fd= socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd < 0) { 
            LOG_ERROR(logger, "socket failed");
            exit(1);
        }
    int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        socklen_t addr_len = sizeof(addr);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(Port);

        if (bind(listen_fd, (sockaddr*)&addr, addr_len) < 0) {
            LOG_ERROR(logger, "bind failed");
            close(listen_fd);
            exit(1);
        }

        if (listen(listen_fd, 10) < 0) {
            LOG_ERROR(logger, "listen failed");

            close(listen_fd);
            exit(1);
        }

        epfd = epoll_create1(0);
        if (epfd == -1) {
            LOG_ERROR(logger, "epoll_create failed");
            close(listen_fd);
            exit(1);
        }

        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = listen_fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
            LOG_ERROR(logger, "epoll_ctl ADD server_fd failed");
            close(listen_fd);
            close(epfd);
            exit(1);
        }








        
    }

    

Server::~Server()
{
    
}