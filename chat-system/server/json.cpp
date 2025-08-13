#include"json.hpp"

// 发送格式4字节长度（网络序+JSON数据
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