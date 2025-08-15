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

// 非阻塞安全的send_json
// int send_json(int sockfd, const json& j) {
//     std::string payload = j.dump();
//     uint32_t len = htonl(payload.size());

//     std::vector<char> buffer(4 + payload.size());
//     memcpy(buffer.data(), &len, 4);
//     memcpy(buffer.data() + 4, payload.data(), payload.size());

//     size_t total_sent = 0;
//     while (total_sent < buffer.size()) {
//         ssize_t n = send(sockfd, buffer.data() + total_sent, buffer.size() - total_sent, 0);
//         if (n > 0) {
//             total_sent += n;
//         }
//         else if (n < 0) {
//             if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 // 缓冲区满了，等待可写
//                 fd_set wfds;
//                 FD_ZERO(&wfds);
//                 FD_SET(sockfd, &wfds);
//                 if (select(sockfd + 1, NULL, &wfds, NULL, NULL) > 0) {
//                     continue; // 重新尝试发送
//                 } else {
//                     return -1; // select 出错
//                 }
//             }
//             return -1; // 其他错误
//         }
//         else {
//             return -1; // 连接关闭
//         }
//     }
//     return 0;
// }
