#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>     //socket()
#include <netdb.h>  //struct hostent
#include <netinet/in.h>  //struct sockaddr_in

#define CONNECT_ERROR -1

#define PORT_NO 5678

void perr(char* message);

void main(int args, char* argv[]) {
    int socketfd;
    struct sockaddr_in serv_addr;
    struct hostent* server_info;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (&socketfd == NULL) {
        perr("Socket build error!");
    }

    // analyze the hostname to find address
    server_info = gethostbyname(argv[1]);
    if (server_info == NULL) {
        perr("Host not found!");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server_info->h_addr, sizeof(server_info->h_addr));
    printf ("this is address:%d",serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(PORT_NO);

    if (connect(socketfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) == CONNECT_ERROR) {
        perr("Host not found!");
    }

    
}

void perr(char* message) {
    printf("%s", message);
    exit(1);
}
