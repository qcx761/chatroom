#include"json.hpp"

// 接收并解析json
int receive_json(int sockfd, json& j) {
    uint32_t len_net = 0;
    ssize_t n = recv(sockfd, &len_net, sizeof(len_net), MSG_WAITALL);
    if (n != sizeof(len_net)) {
        std::cerr << "Failed to read length prefix.\n";
        return -1;
    }

    uint32_t len = ntohl(len_net); // 转换为主机字节序
    std::vector<char> buffer(len);

    n = recv(sockfd, buffer.data(), len, MSG_WAITALL);
    if (n != (ssize_t)len) {
        std::cerr << "Failed to read full JSON body.\n";
        return -1;
    }

    try {
        j = json::parse(buffer.begin(), buffer.end());
    } catch (std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}


int send_json(int sockfd, const json& j) {
    std::string data = j.dump();
    uint32_t len = htonl(data.size()); // 先转网络字节序

    if (send(sockfd, &len, sizeof(len), 0) != sizeof(len)) {
        std::cerr << "Failed to send length prefix.\n";
        return -1;
    }

    if (send(sockfd, data.c_str(), data.size(), 0) != (ssize_t)data.size()) {
        std::cerr << "Failed to send JSON body.\n";
        return -1;
    }

    return 0;
}
