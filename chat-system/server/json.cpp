#include"json.hpp"

// 发送格式：4字节长度（网络序）+ JSON 数据
int send_json(int sockfd, const json& j) {
    std::string payload = j.dump();
    uint32_t len = htonl(payload.size());

    std::vector<char> buffer(4 + payload.size());
    memcpy(buffer.data(), &len, 4);
    memcpy(buffer.data() + 4, payload.data(), payload.size());

    size_t total_sent = 0;
    while (total_sent < buffer.size()) {
        ssize_t n = send(sockfd, buffer.data() + total_sent, buffer.size() - total_sent, 0);
        if (n <= 0) return -1;  // 发送失败或连接关闭
        total_sent += n;
    }

    return 0;
}

// int receive_json(int sockfd, json& j) {
//     char buf[4096];
//     while (true) {
//         ssize_t n = recv(sockfd, buf, sizeof(buf), 0);
//         if (n > 0) {
//             fd_buffers[sockfd] += std::string(buf, n);  // 直接拼接
//         } else if (n == 0) {
//             return -1; // 连接关闭
//         } else {
//             if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 break; // 读完
//             }
//             return -1; // 其他错误
//         }
//     }

//     auto& buffer = fd_buffers[sockfd];
//     while (true) {
//         if (buffer.size() < 4) return 1; // 长度字段不够

//         uint32_t len_net = 0;
//         memcpy(&len_net, buffer.data(), 4);
//         uint32_t len = ntohl(len_net);

//         if (len == 0 || len > 10 * 1024 * 1024) return -1; // 长度异常

//         if (buffer.size() < 4 + len) return 1; // 数据不完整

//         std::string json_str = buffer.substr(4, len);

//         try {
//             j = json::parse(json_str);
//         } catch (...) {
//             return -1;
//         }

//         // 删除已处理数据
//         buffer.erase(0, 4 + len);

//         return 0; // 成功
//     }
// }




// int receive_json(int sockfd, json& j) {
//     // 1. 读取长度字段（4 字节）
//     uint32_t len_net = 0;
//     size_t received = 0;
//     char* len_ptr = reinterpret_cast<char*>(&len_net);

//     while (received < 4) {
//         ssize_t n = recv(sockfd, len_ptr + received, 4 - received, 0);
//         if (n <= 0) return -1;  // 出错或连接关闭
//         received += n;
//     }

//     uint32_t len = ntohl(len_net);
//     if (len == 0 || len > 10 * 1024 * 1024) return -1;  // 防止异常数据包

//     // 2. 读取 JSON 数据体
//     std::vector<char> json_buf(len);
//     received = 0;

//     while (received < len) {
//         ssize_t n = recv(sockfd, json_buf.data() + received, len - received, 0);
//         if (n <= 0) return -1;  // 出错或连接关闭
//         received += n;
//     }

//     try {
//         j = json::parse(std::string(json_buf.begin(), json_buf.end()));
//     } catch (const std::exception& e) {
//         std::cerr << "JSON parse error: " << e.what() << std::endl;
//         return -1;
//     }

//     return 0;
// }
