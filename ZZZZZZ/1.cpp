
// 非阻塞接收json，返回值：0成功收到并解析一条json，1数据不完整需继续接收，-1错误或断开
int receive_json(int sockfd, json& j) {
    // 静态缓冲区用于缓存未完整读取的数据，单socket单线程安全
    static std::vector<char> buffer;

    char temp[4096];
    while (true) {
        ssize_t n = recv(sockfd, temp, sizeof(temp), 0);
        if (n > 0) {
            buffer.insert(buffer.end(), temp, temp + n);
        } else if (n == 0) {
            // 连接关闭
            return -1;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞下无更多数据，退出读取循环
                break;
            } else if (errno == EINTR) {
                // 中断，继续接收
                continue;
            } else {
                perror("recv");
                return -1;
            }
        }
    }

    // 解析完整消息
    while (true) {
        if (buffer.size() < 4) {
            // 长度字段不完整，等待更多数据
            return 1;
        }

        uint32_t len_net = 0;
        memcpy(&len_net, buffer.data(), 4);
        uint32_t len = ntohl(len_net);

        if (buffer.size() < 4 + len) {
            // json内容不完整，继续等待数据
            return 1;
        }

        std::string json_str(buffer.begin() + 4, buffer.begin() + 4 + len);

        try {
            j = json::parse(json_str);
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return -1;
        }

        // 消息处理完毕，移除缓冲区已处理部分
        buffer.erase(buffer.begin(), buffer.begin() + 4 + len);

        return 0;  // 成功解析一条消息
    }

    return 1; // 理论上不会到这里
}
