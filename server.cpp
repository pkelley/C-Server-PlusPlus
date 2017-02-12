#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

#define PORT "1776"  // the port users will be connecting to

#define BACKLOG 10     // # of pending connections queue will hold

#define MAXDATASIZE 1024 // max number of bytes we can get at once


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);
    //restore errno
    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int server, new_fd;  // listen on sock_fd, new connection on new_fd server
    struct addrinfo hints, *servinfo, *p; //p will iterate over result in call to getaddrinfo
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((server = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(server, p->ai_addr, p->ai_addrlen) == -1) {
            close(server);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(server, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    bool dontQuit = true;
    char buf[MAXDATASIZE];

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(server, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(server); // child doesn't need the listener
            if (send(new_fd, "Now Connected", 13, 0) == -1)
                perror("send");

            while(dontQuit){
                recv(new_fd, buf, MAXDATASIZE-1, 0);
                if(*buf == '#')
                    dontQuit = false;
                cout << buf << endl;
            }

            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
/*#include <sstream>
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

    if(inet_pton(AF_INET, "10.12.110.57", &(server_addr.sin_addr)) < 0){ // IPv4
        cout<<"Error converting IP\n";
        return(1);
    }

    return 0;

}*/
