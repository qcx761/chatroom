#include "json.hpp"

using namespace std;




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
