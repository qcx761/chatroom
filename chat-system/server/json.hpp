#pragma once

#include<iostream>
#include <string>
#include <nlohmann/json.hpp> 
#include <sys/socket.h>
#include <arpa/inet.h>


using json = nlohmann::json;


int send_json(int sock,const json& j);
int receive_json(int sock,json& j);





