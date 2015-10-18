#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include "packet.h"

#define BIND_ERROR -1
#define ACCEPT_ERROR -1

#define PORT_NO 5678

int newsocketfd;

void printError(char*);
void createFile(char*);
void editFile(char*);
void removeFile(char*);
void listFile();
void downloadFile(char*);

void main() {
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
    newsocketfd = accept(socketfd, (struct sockaddr*) &cli_addr,&clilen);
    if (newsocketfd == ACCEPT_ERROR) {  //if accept is error
        printError("Socket accept error!");
    }
    else {
        printf("the client IP is: %d\n", cli_addr.sin_addr.s_addr);
    }
    while (1) {
        if (readpacket(newsocketfd) == 0) {
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
                    downloadFile(buffer.content);
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
}

void createFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 6);
    memset(command, 0, sizeof(strlen(content) + 6));
    strcat(command, "touch ");
    strcat(command, content);
    printf("%s", command);
    if (system(command) != 0) {
        printError("Create File Error!");
    }
    else {
        printf("File %s created\n", content);
    }
    free(command);
}

void editFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 8);
    memset(command, 0, sizeof(strlen(content) + 8));
    strcat(command, "test -e ");
    strcat(command, content);

    if (system(command) != 0) {
        printf("File not exist!\n");
        sendpacket(newsocketfd, 'E', "no");
    }
    else {
        printf("File %s found\n", content);
        FILE* openFile;
        openFile = fopen(content, "w");
        sendpacket(newsocketfd, 'E', "ok");
        readpacket(newsocketfd);
        fputs(buffer.content, openFile);
        fclose(openFile);
    }
    free(command);
}

void removeFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 3);
    memset(command, 0, sizeof(strlen(content) + 3));
    strcat(command, "rm ");
    strcat(command, content);

    if (system(command) != 0) {
        printError("Remove File Error!");
    }
    else {
        printf("File %s removed\n", content);
    }
    free(command);
}

void listFile() {
    DIR* d;
    struct dirent* dir;
    d = opendir(".");
    char* file_list = (char*)malloc(1024);
    memset(file_list, 0, 1024);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            strncat(file_list, dir->d_name, sizeof(dir->d_name));
            strncat(file_list, "\n", 1);
            //printf("%s\n", dir->d_name);
        }
        sendpacket(newsocketfd, 'L', file_list);
        closedir(d);
    }
}

void downloadFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 8);
    memset(command, 0, sizeof(strlen(content) + 8));
    strcat(command, "test -e ");
    strcat(command, content);

    if (system(command) != 0) {
        printf("File not exist!\n");
        sendpacket(newsocketfd, 'N', "no");
    }
    else {
        printf("File %s found\n", content);
        FILE* openFile;
        openFile = fopen(content, "r");
        sendpacket(newsocketfd, 'Y', "ok");
        char* piece = (char*)malloc(1024);
        memset(piece, 0, 1024);
        while (fgets(piece, 1024, openFile) != NULL) {
            sendpacket(newsocketfd, 'D', piece);
            memset(piece, 0, 1024);
        }
        sendpacket(newsocketfd, 'Y', "end");
        fclose(openFile);
        free(piece);
    }
    free(command);
}

void printError(char* message) {
    printf("%s\n", message);
    exit(1);
}
