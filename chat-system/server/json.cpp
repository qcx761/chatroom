#include"json.hpp"

// 发送json，消息格式: 4字节长度+json字符串
int send_json(int sockfd, const json& j) {
    std::string payload = j.dump();
    uint32_t len = htonl(payload.size());  // 转为网络字节序

    // 组合长度和内容
    std::vector<char> buffer(4 + payload.size());
    memcpy(buffer.data(), &len, 4);
    memcpy(buffer.data() + 4, payload.c_str(), payload.size());

    ssize_t sent = send(sockfd, buffer.data(), buffer.size(), 0);
    return (sent == (ssize_t)buffer.size()) ? 0 : -1;
}


int receive_json(int sockfd, json& j) {
    char len_buf[4];
    ssize_t n = recv(sockfd, len_buf, 4, MSG_WAITALL);
    if (n <= 0) return -1;

    uint32_t len;
    memcpy(&len, len_buf, 4);
    len = ntohl(len);  // 转为主机字节序

    std::vector<char> json_buf(len);
    n = recv(sockfd, json_buf.data(), len, MSG_WAITALL);
    if (n <= 0) return -1;

    try {
        j = json::parse(std::string(json_buf.begin(), json_buf.end()));
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}



// int send_json(int sockfd, const json& j) {
//     std::string data = j.dump();
//     uint32_t len = htonl(data.size()); // 先转网络字节序

//     if (send(sockfd, &len, sizeof(len), 0) != sizeof(len)) {
//         std::cerr << "Failed to send length prefix.\n";
//         return -1;
//     }

//     if (send(sockfd, data.c_str(), data.size(), 0) != (ssize_t)data.size()) {
//         std::cerr << "Failed to send JSON body.\n";
//         return -1;
//     }

//     return 0;
// }

// // 接收并解析json
// int receive_json(int sockfd, json& j) {
//     uint32_t len_net = 0;
//     ssize_t n = recv(sockfd, &len_net, sizeof(len_net), MSG_WAITALL);
//     if (n != sizeof(len_net)) {
//         std::cerr << "Failed to read length prefix.\n";
//         return -1;
//     }

//     uint32_t len = ntohl(len_net); // 转换为主机字节序
//     std::vector<char> buffer(len);

//     n = recv(sockfd, buffer.data(), len, MSG_WAITALL);
//     if (n != (ssize_t)len) {
//         std::cerr << "Failed to read full JSON body.\n";
//         return -1;
//     }

//     try {
//         j = json::parse(buffer.begin(), buffer.end());
//     } catch (std::exception& e) {
//         std::cerr << "JSON parse error: " << e.what() << std::endl;
//         return -1;
//     }

//     return 0;
// }











// // 发送json，消息格式: 4字节长度+json字符串
// int send_json(int sockfd, const json& j) {
//     std::string payload = j.dump();
//     uint32_t len = htonl(payload.size());  // 转为网络字节序

//     // 组合长度和内容
//     std::vector<char> buffer(4 + payload.size());
//     memcpy(buffer.data(), &len, 4);
//     memcpy(buffer.data() + 4, payload.c_str(), payload.size());

//     ssize_t sent = send(sockfd, buffer.data(), buffer.size(), 0);
//     return (sent == (ssize_t)buffer.size()) ? 0 : -1;
// }


// // 非阻塞接收json，返回值：0成功收到并解析一条json，1数据不完整需继续接收，-1错误或断开
// int receive_json(int sockfd, json& j) {
//     // 静态缓冲区用于缓存未完整读取的数据，单socket单线程安全
//     static std::vector<char> buffer;

//     char temp[4096];
//     while (true) {
//         ssize_t n = recv(sockfd, temp, sizeof(temp), 0);
//         if (n > 0) {
//             buffer.insert(buffer.end(), temp, temp + n);
//         } else if (n == 0) {
//             // 连接关闭
//             return -1;
//         } else {
//             if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 // 非阻塞下无更多数据，退出读取循环
//                 break;
//             } else if (errno == EINTR) {
//                 // 中断，继续接收
//                 continue;
//             } else {
//                 perror("recv");
//                 return -1;
//             }
//         }
//     }

//     // 解析完整消息
//     while (true) {
//         if (buffer.size() < 4) {
//             // 长度字段不完整，等待更多数据
//             return 1;
//         }

//         uint32_t len_net = 0;
//         memcpy(&len_net, buffer.data(), 4);
//         uint32_t len = ntohl(len_net);

//         if (buffer.size() < 4 + len) {
//             // json内容不完整，继续等待数据
//             return 1;
//         }

//         std::string json_str(buffer.begin() + 4, buffer.begin() + 4 + len);

//         try {
//             j = json::parse(json_str);
//         } catch (const std::exception& e) {
//             std::cerr << "JSON parse error: " << e.what() << std::endl;
//             return -1;
//         }

//         // 消息处理完毕，移除缓冲区已处理部分
//         buffer.erase(buffer.begin(), buffer.begin() + 4 + len);

//         return 0;  // 成功解析一条消息
//     }

//     return 1; // 理论上不会到这里
// }

