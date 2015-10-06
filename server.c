#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BIND_ERROR -1
#define ACCEPT_ERROR -1

#define PORT_NO 5678

int main() {
    struct sockaddr_in serv_addr, cli_addr;

    // flush serv_addr with zero
    bzero((char*) &serv_addr, sizeof(serv_addr));

    // write info into serv_addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NO);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // bind socket
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    int binding = bind (socketfd, (struct sock_addr*) &serv_addr, sizeof(serv_addr));
    if (binding == BIND_ERROR) {    //if binding error
        printf("Socket binding error!");
    }
    else {
        listen (socketfd, 1);
    }

    int newsocketfd = accept(socketfd, (struct sock_addr*) &cli_addr, sizeof(cli_addr));
    if (newsocketfd == ACCEPT_ERROR) {  //if accepting error
        printf("Socket accept error!");
    }
    else {
        printf("the client IP is: %d", cli_addr.sin_addr.s_addr);
    }
}
