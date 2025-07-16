#include "msg.hpp"
#include "json.hpp"

using json = nlohmann::json;

// 处理服务端返回的错误消息
void error_msg(int fd, const json &response) {
    
    
    std::cerr << "[ERROR] 服务器返回错误: " << response.value("msg", "未知错误") << std::endl;
    
    
    // 可以做其他错误处理，比如弹窗、日志记录等
}

// 处理服务端返回的登录响应
bool log_in_msg(int fd, const json &response) {
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[登录成功] " << msg << std::endl;
        std::string token = response.value("token", "");
        if (!token.empty()) {
            // 保存token，供后续请求使用
            // 例如保存到客户端变量或文件
            std::cout << "收到Token: " << token << std::endl;
        }
        return true;
    } else if (status == "fail") {
        std::cout << "[登录失败] " << msg << std::endl;
        return false;
    } else {
        std::cerr << "[登录错误] " << msg << std::endl;
        return false;
    }
}

// 处理服务端返回的注册响应
void sign_up_msg(int fd, const json &response) {
    std::string status = response.value("status", "error");
    std::string msg = response.value("msg", "未知错误");

    if (status == "success") {
        std::cout << "[注册成功] " << msg << std::endl;
        // 注册成功后，可以自动登录或提示用户去登录
    } else if (status == "fail") {
        std::cout << "[注册失败] " << msg << std::endl;
    } else {
        std::cerr << "[注册错误] " << msg << std::endl;
    }
}
