#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

int main(){

    struct sockaddr_storage their_addr;
    struct addrinfo hints, *res;
    int client;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, "1776", &hints, &res);

    // make a socket:

    client = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(client < 0){
        cout<<"Error w/ client connection\n";
        return(1);
    }
    cout<<"Client socket created\n";

    // bind it to the port we passed in to getaddrinfo():

    if (bind(client, res->ai_addr, res->ai_addrlen) < 0){ //check binding error and cast server_addr to sockaddr for bind call
        cout<<"Error binding socket to server address\n";
        return(1);
    }

    bool exit = false;
    //buffer for messages
    int bufferSize = 1024;
    char buffer[bufferSize];


    cout<<"Listening for clients....\n";

    //listenm with # of connections allowed equal to 1

    if (listen(client, 1) < 0){ //check listen error
        cout<<"Error listening.\n";
        return(1);
    }
    socklen_t addr_size;
    addr_size = sizeof their_addr;
    int server = accept(client, res->ai_addr, &addr_size);

    while(server > 0){
        strcpy(buffer, "Server connected!!\n");
        send(server, buffer, bufferSize, 0);

        cout<< "Connected with client!\n";
        cout<< "Enter $ to end connection\n";
        cout<< "Client: ";
        do{
            recv(server, buffer, bufferSize, 0);
            cout << buffer << " ";
            if (*buffer == '$'){
                *buffer = '*';
                exit = true;
            }
        } while (*buffer != '*');
        do{
            cout<<"\nServer: ";
            do{
                cin >> buffer;
                send(server, buffer, bufferSize, 0);
                if (*buffer == '$'){
                    *buffer = '*';
                    exit = true;
                }
            } while(*buffer != '*');
        }while(!exit);
    }
/*
    if(inet_pton(AF_INET, "10.12.110.57", &(server_addr.sin_addr)) < 0){ // IPv4
        cout<<"Error converting IP\n";
        return(1);
    }*/

    return 0;

}
