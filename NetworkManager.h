#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H
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

class NetworkManager 
{
public:

	int setAddrInfo();
	int bindTo();
	int listenTo(int backlog);
	int reapKids(void (&f)(int));
	void *get_in_addr(struct sockaddr *sa);
	int acceptConns();


private:
	int server, new_fd;  // listen on server, new connection on new_fd server
    struct addrinfo hints, *servinfo, *p; //p will iterate over result in call to getaddrinfo
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;


};
#endif
