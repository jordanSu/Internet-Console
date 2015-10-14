#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "packet.h"

#define BIND_ERROR -1
#define ACCEPT_ERROR -1

#define PORT_NO 5678

struct packet_content buffer;
char* packet;

int main() {
    struct sockaddr_in serv_addr, cli_addr;
    packet = (unsigned char*)malloc(sizeof(buffer));
    int receive_choice;

    // flush serv_addr with zero
    memset((char*) &serv_addr, 0, sizeof(serv_addr));

    // write info into serv_addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NO);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // bind socket
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    int binding = bind (socketfd, (struct sockaddr*) &serv_addr,(socklen_t) sizeof(serv_addr));
    if (binding == BIND_ERROR) {    //if bind is error
        printf("Socket binding error!");
    }
    else {
        listen (socketfd, 1);
    }
    socklen_t clilen = sizeof(cli_addr);
    int newsocketfd = accept(socketfd, (struct sockaddr*) &cli_addr,&clilen);
    if (newsocketfd == ACCEPT_ERROR) {  //if accept is error
        printf("Socket accept error!");
    }
    else {
        printf("the client IP is: %d\n", cli_addr.sin_addr.s_addr);
    }
    while (read(newsocketfd, packet, sizeof(buffer))) {
        memset(&buffer, 0, sizeof(buffer));
        memcpy(&buffer, packet, sizeof(buffer));
        receive_choice = buffer.command;
        switch (receive_choice) {
            case 'C':
                printf("Hi");
            default:
                printf("No good!");
        }
    }
    close(socketfd);
    close(newsocketfd);
    return 0;   //main() return 0
}
