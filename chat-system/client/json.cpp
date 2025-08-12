#include "json.hpp"

// 4字节长度+JSON数据
int send_json(int sockfd, const json& j) {
    std::string payload = j.dump();
    uint32_t len = htonl(payload.size());

    std::vector<char> buffer(4 + payload.size());
    memcpy(buffer.data(), &len, 4);
    memcpy(buffer.data() + 4, payload.data(), payload.size());

    size_t total_sent = 0;
    while (total_sent < buffer.size()) {
        ssize_t n = send(sockfd, buffer.data() + total_sent, buffer.size() - total_sent, 0);
        if (n <= 0){
            return -1;
        }  // 发送失败或连接关闭
        total_sent += n;
    }

    return 0;
}

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
