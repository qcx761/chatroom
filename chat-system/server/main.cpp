#include"server.hpp"

int main(int argc, char* argv[]) {
    
    std::string port_str(argv[1]);
    int port = std::stoi(port_str);

    Server Server(port);

    return 0;
}