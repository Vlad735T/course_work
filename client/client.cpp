#include <iostream>
#include <string>
#include <cstring>

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
using json = nlohmann::json;



int main() {


    string path_to_json =  "/home/kln735/course_work/server/configuration.json";
    cout << "Current directory: " << path_to_json << endl;

    ifstream json_file(path_to_json);
    if(!json_file.is_open()){  
        cerr << "Error opening json file\n";
        exit(-1);
    }

    json config;
    json_file >> config;

    string host = config["client"]["host"].get<string>();
    int PORT = config["client"]["port"];



    int sock = 0;
    struct sockaddr_in serv_addr;

    const char* message_client = "Hello from client";
    char bufer[1024] = {0};


    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cerr << "Socket creation failed\n";
        exit (-1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

// Преобразование IP-адреса
    if(inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0){
        cerr << "Invalid address\n";
        exit(-1);
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        cerr << "Connection Failed\n";
        exit(-1);
    }

    send(sock, message_client, strlen(message_client), 0);
    cout << "Message sent to server" << endl;

    read(sock, bufer, 1024);
    cout << "Response from server: " << bufer << endl;

    close(sock);

    return 0;
}
