#include "ftpserver.hpp"
#include <iostream>

int main() {
    FTPServer server(2100);  
    if (!server.init()) {
        std::cerr << "Failed to initialize server." << std::endl;
        return 1;
    }
    std::cout << "FTP Server running on port 2100" << std::endl;
    server.run();
    return 0;
}
