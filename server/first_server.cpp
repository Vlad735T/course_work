#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include <fstream>

#include <unordered_map>

#include <nlohmann/json.hpp>
#include <filesystem>

#include <cstdint>
#include <cstring>

#include <sys/types.h> // for type definitions
#include <sys/socket.h> // for socket operation
#include <netinet/in.h> // for network adress ( ipv4, ipv6 )
#include <unistd.h> // for bridge betweenn Linux and programm
#include <arpa/inet.h> // for inet_pton function

using namespace std;

using json = nlohmann::json;



int main(){

    filesystem::path current_path = filesystem::current_path();
    cout << "Current directory: " << current_path << endl;

    string paths = current_path;
    string file_path =  paths + "/configuration.json";
    cout << "Current directory: " << file_path << endl;

    ifstream json_file(file_path);
    if(!json_file.is_open()){
        cerr << "Error opening json file\n";
        exit(-1);
    }

    json config;
    json_file >> config;

    string server_host = config["server"]["host"];
    int PORT = config["server"]["port"];

    cout << "Server will run on " << server_host << " with port: " << PORT << endl;

//socket servera
    int server_fd, new_socket; // for take zapross from client  listen port and for obsheniya s client
    struct sockaddr_in address;
    int opt = 1; // for nastrouki  socket
    int addr_len = sizeof(address);
    char bufer[1024] = {0};

    const char* answer = "Hello from server"; 

    // Creating socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ //ipv4 , tcp protocol
        cerr << "Socket not create!!!!\n";
        exit(-1);
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){ // for nastroiki parametrov socketa serv
        cerr << "Setsockopt error\n";
        exit(-1);
    }

    address.sin_family = AF_INET; // ipv4
    address.sin_addr.s_addr = inet_addr(server_host.c_str()); // prinimaem soedinenie from localhost
    address.sin_port = htons(PORT); //  remake port int network poryadok byte for coorect peredachi between ystroistvami

    // Bind the socket to the address and port
    if(bind(server_fd, (struct sockaddr *)& address, addr_len) < 0){
        cerr << "Bind failed\n";
        exit(-1);
    }

    if(listen(server_fd, 3) < 0){
        cerr << "Listen failed\n";
        exit(-1);
    }


    while(true){

        cout << "Waiting for a client connection...\n";

        string input;
        getline(cin, input);

       // Handle server commands and log events
        if (input == "exit") {
            cout << "Shutting down server...\n";
            break;  // Exit the loop to stop the server
        } else if (input == "123") {
            cout << "Received command '123'. Performing action...\n";
        } else if (input == "456") {
            cout << "Received command '456'. Performing action...\n";
        } else if (input == "789") {
            cout << "Received command '789'. Performing action...\n";
        } 

    // kak only client podkluch we srazy listem his i videlyaem dor them socket
        if((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t *)&addr_len )) < 0){
            cerr << "Accept failed\n";
            exit(-1);
        }

        cout << "Client  connected: " << inet_ntoa(address.sin_addr)  << "\n";

        read(new_socket, bufer, 1024);
        cout << "Message from client: " << bufer << "\n";

        send(new_socket, answer, strlen(answer), 0);
        cout << "Message send to client\n";

    }
    
    close(new_socket);

    close(server_fd);

    return 0;
}