bool init() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) { 
            perror("socket failed");
            return false;
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        socklen_t addr_len = sizeof(addr);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(control_port);

        if (bind(server_fd, (sockaddr*)&addr, addr_len) < 0) {
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

    void run() {
        epoll_event events[MAX_EVENTS];
        while (is_running) {
            int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
            if (n == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("epoll_wait");
                break;
            }
            for (int i = 0; i < n; i++) {
                int fd = events[i].data.fd;
                uint32_t evs = events[i].events;

                if ((evs & EPOLLERR) || (evs & EPOLLHUP) || (evs & EPOLLRDHUP)) {
                    close_connection(fd);
                    continue;
                }

                if (fd == server_fd) { // 客户端连接
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("accept");
                            break;
                        }
                    }

                    set_nonblocking(client_fd);
                    epoll_event client_ev{};
                    client_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR;
                    client_ev.data.fd = client_fd;

                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
                        perror("epoll_ctl ADD client_fd failed");
                        close(client_fd);
                    }

                    cout << "New control connection accepted: fd=" << client_fd << endl;
                } else {
                    int port = get_socket_local_port(fd);

                    if (port == CONTROL_PORT) { // 控制连接
                        cout << "控制连接进入" << endl;
                        handle_control_fd(fd);
                    } else { // 数据连接
                        cout << "数据连接进入" << endl;

                        // 从数据连接获取控制连接fd
                        int control_fd = group.get_control_from_data(fd); // 能不能获得控制fd，不能就是数据监听fd
                        if (control_fd == -1) {
                            // 可能是PASV模式的监听套接字
                            control_fd = group.get_control_from_listen(fd); // 数据监听fd获取控制fd
                            if (control_fd != -1) {
                                // 有新的数据连接到来
                                sockaddr_in client_addr{};
                                socklen_t client_len = sizeof(client_addr);
                                int data_fd = accept(fd, (sockaddr*)&client_addr, &client_len);
                                if (data_fd == -1) {
                                    perror("accept data connection failed");
                                    continue;
                                }

                                set_nonblocking(data_fd);
                                epoll_event data_ev{};
                                data_ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLERR;
                                data_ev.data.fd = data_fd;

                                if (epoll_ctl(epfd, EPOLL_CTL_ADD, data_fd, &data_ev) == -1) {
                                    perror("epoll_ctl ADD data_fd failed");
                                    close(data_fd);
                                    continue;
                                }

                                // 关联数据连接和控制连接
                                group.bind_data_to_control(data_fd, control_fd);
                                cout << "New data connection accepted for control fd: " << control_fd << ", data fd: " << data_fd << endl;

                                // 不要在这里关闭监听套接字，因为数据传输可能还没有开始
                                // group.unbind_control_from_listen(fd);
                                // close(fd);

                                // 要保留监听套接字，数据传输可能还没有开始
                                // 监听套接字的清理应该在数据传输完成后进行
                            } else {
                                cerr << "No control connection associated with data fd: " << fd << endl;
                                close_connection(fd);
                            }
                        } else {
                            // 正常的数据连接处理
                            cout << "处理数据连接中...." << endl;
                            int command = group.get_command_type(control_fd);
                            string filename = group.get_filename(control_fd);

                            thread_pool.enqueue([this, fd, command, filename]() {
                                handle_data_connection(fd, command, filename);
                            });

                            // 删除控制连接和数据链接的映射
                            group.unbind_control_from_data(fd);

                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr); // 结束数据连接
                        }
                    }
                }
            }
        }
    }
