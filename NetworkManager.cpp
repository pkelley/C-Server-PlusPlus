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
#include "NetworkManager.h"

using namespace std;

#define PORT "1776"  // the port users will be connecting to

#define MAXDATASIZE 1024 // max number of bytes we can get at once



int NetworkManager::setAddrInfo(){
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    return 0;
}

int NetworkManager::bindTo(){
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
            return 1;
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
        return 1;
    }
    return 0;
}

int NetworkManager::listenTo(int backlog){
	if (listen(server, backlog) == -1) {
        perror("listen");
        return 1;
    }
    return 0;
}


int NetworkManager::reapKids(void (&f)(int)) {
	sa.sa_handler = f; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    return 0;
}

void* NetworkManager::get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int NetworkManager::acceptConns(){
	cout << "server: waiting for connections...\n";

	bool dontQuit = true;
    char buf[MAXDATASIZE];
    int numByte = 0;

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
                if ((numByte = recv(new_fd, buf, MAXDATASIZE-1, 0)) < 1){
                	if (numByte == 0){
                		cout << "Lost connection to client "<< s << endl;
                		dontQuit = false;
                	}
                	else{
                		cout << "Error in connection to client "<< s << endl;
                		dontQuit = false;
                	}
                	close(new_fd);
                }
                if(*buf == '#'){
                    dontQuit = false;
                	close(new_fd);
                }
                cout << buf << endl;
            }

            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }
    return 0;
}
















