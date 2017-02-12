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

#define BACKLOG 10     // # of pending connections queue will hold

void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);
    //restore errno
    errno = saved_errno;
}

int main(void)
{
    NetworkManager NetMan;

    if (NetMan.setAddrInfo() != 0){
        cout << "Error setting addr info\n";
    }

    if(NetMan.bindTo() != 0){
        cout<< "Error binding socket\n";
    }

    if(NetMan.listenTo(BACKLOG) != 0){
        cout<< "Error listening\n";
    }

    if(NetMan.reapKids(sigchld_handler) != 0){
        cout<< "Error reaping child\n";
    }

    if(NetMan.acceptConns() != 0){
        cout<< "Error reaping child\n";
    }


    return 0;
}