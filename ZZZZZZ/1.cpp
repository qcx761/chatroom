Server::Server(int port) : events(10), pool(20) {
    // redisManager = RedisManager{};

    //创建套接字
    listening_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    //设置端口复用
    int optval = 1;
    if (setsockopt(listening_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(listening_sockfd);
        exit(EXIT_FAILURE);
    }

    //初始化套接字协议地址结构
    addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    //绑定地址
    if (bind(listening_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LogFatal("bind失败了 快去设置socket reuse");
    }

    //监听连接
    if (listen(listening_sockfd, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    //设置套接字为非阻塞模式
    int flags = fcntl(listening_sockfd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl(F_GETFL) failed" << std::endl;
        return;
    }
    if (fcntl(listening_sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl(F_SETFL) failed" << std::endl;
    }

    //日志输出启动服务器
    LogInfo("成功启动服务器!");

    //创建epoll
    epfd = epoll_create1(0);
    if (epfd < 0) {
        throw std::runtime_error("Failed to create epoll instance");
    }
    
    //注册套接字到epoll（ET 模式），用于描述希望epoll监视的事件
    event.events = EPOLLIN | EPOLLET; // 读事件以及边缘触发模式
    event.data.fd = listening_sockfd;

    //监视事件
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listening_sockfd, &event) < 0) {
        throw std::runtime_error("Failed to add socket to epoll");
    }

    while (1) {
        //等待事件并处理，将事件添加到事件数组中
        int num_events = epoll_wait(epfd, events.data(), events.size(), -1);
        if (num_events < 0) {
            std::cerr << "epoll_wait failed" << std::endl;
            continue;
        }

        //遍历事件数组
        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == listening_sockfd) {
                //接收连接
                socklen_t addrlen = sizeof(addr);
                int connected_sockfd = accept(listening_sockfd, (struct sockaddr *)&addr, &addrlen);
                if (connected_sockfd < 0) {
                    std::cerr << "Accept failed" << std::endl;
                }

                //日志输出连接到客户端
                LogInfo("成功连接到客户端!");
                // LogInfo("connected_socked = {}", connected_sockfd);

                //设置套接字为非阻塞模式
                int flags = fcntl(connected_sockfd, F_GETFL, 0);
                if (flags == -1) {
                    std::cerr << "fcntl(F_GETFL) failed" << std::endl;
                    return;
                }
                if (fcntl(connected_sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
                    std::cerr << "fcntl(F_SETFL) failed" << std::endl;
                }

                //注册套接字到epoll（ET 模式），用于描述希望epoll监视的事件
                event2.events = EPOLLIN | EPOLLET; // 读事件以及边缘触发模式
                event2.data.fd = connected_sockfd;

                //监视事件
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, connected_sockfd, &event2) < 0) {
                    throw std::runtime_error("Failed to add socket to epoll");
                }
                
                heartbeat_map[connected_sockfd] = 1; 
                pool.add_task([this, connected_sockfd] {
                    // LogTrace("something read happened");
                    heartbeat(connected_sockfd);
                });

            } else if (events[i].events & EPOLLIN) {

                //如果监控到读事件就将接收函数添加到线程池运行
                pool.add_task([this, fd = events[i].data.fd] {
                    // LogTrace("something read happened");
                    do_recv(fd);
                });
                
            } else if (events[i].events & EPOLLOUT) {
                
            } else if (events[i].events & EPOLLERR) {

            }
        }
    }
}
