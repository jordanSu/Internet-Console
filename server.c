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

void printError(char*);
void createFile(char*);
void editFile(char*);
void removeFile(char*);
void listFile();
void downloadFile();

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
        printError("Socket binding error!");
    }
    else {
        listen (socketfd, 1);
    }
    socklen_t clilen = sizeof(cli_addr);
    int newsocketfd = accept(socketfd, (struct sockaddr*) &cli_addr,&clilen);
    if (newsocketfd == ACCEPT_ERROR) {  //if accept is error
        printError("Socket accept error!");
    }
    else {
        printf("the client IP is: %d\n", cli_addr.sin_addr.s_addr);
    }
    while (1) {
        if (read(newsocketfd, packet, sizeof(buffer)) > 0) {
            memset(&buffer, 0, sizeof(buffer));
            memcpy(&buffer, packet, sizeof(buffer));
            receive_choice = buffer.command;
            switch (receive_choice) {
                case 'C':
                    createFile(buffer.content);
                    break;
                case 'E':
                    editFile(buffer.content);
                    break;
                case 'R':
                    removeFile(buffer.content);
                    break;
                case 'L':
                    listFile();
                    break;
                case 'D':
                    downloadFile();
                    break;
                default:
                    printf("Not good!");
            }
        }
        else {
            break;
        }
    }
    close(socketfd);
    close(newsocketfd);
    return 0;   //main() return 0
}

void createFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 6);
    strcat(command, "touch ");
    strcat(command, content);

    if (system(command) != 0) {
        printError("Create File Error!");
    }
    else {
        printf("File %s created\n", content);
    }
}

void editFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 8);
    strcat(command, "test -e ");
    strcat(command, content);

    //strcat(command, " ]")

    if (system(command) != 0) {
        printf("File not exist!");
    }
    else {
        printf("File %s found\n", content);
    }
}

void removeFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 3);
    strcat(command, "rm ");
    strcat(command, content);

    if (system(command) != 0) {
        printError("Remove File Error!");
    }
    else {
        printf("File %s removed\n", content);
    }
}

void listFile() {

}

void downloadFile() {

}

void printError(char* message) {
    printf("%s\n", message);
    exit(1);
}
