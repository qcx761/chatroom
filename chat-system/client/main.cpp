#include<iostream>
#include"client.hpp"

int main(int argc, char* argv[]) {

    std::string ip = argv[1];


    std::string port_str(argv[2]);
    int port = std::stoi(port_str);
    
    Client client(ip, port);

    client.start();
    // client.stop();  
    
    return 0;
}


