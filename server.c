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
void zipFile(char*);
void unzipFile(char*);
void encryptFile(char*);
void decryptFile(char*);

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
                case 'Z':
                    if (strcmp(buffer.content, "zip") == 0) {
                        readpacket(newsocketfd);
                        zipFile(buffer.content);
                    }
                    else if (strcmp(buffer.content, "unzip") == 0) {
                        readpacket(newsocketfd);
                        unzipFile(buffer.content);
                    }
                    break;
                case 'S':
                    if (strcmp(buffer.content, "encrypt") == 0) {
                        readpacket(newsocketfd);
                        encryptFile(buffer.content);
                    }
                    else if (strcmp(buffer.content, "decrypt") == 0) {
                        readpacket(newsocketfd);
                        decryptFile(buffer.content);
                    }
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
        printError("Create File %s Error!");
        sendpacket(newsocketfd, 'C', "no");
    }
    else {
        printf("File %s created\n", content);
        sendpacket(newsocketfd, 'C', "ok");
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
        sendpacket(newsocketfd, 'R', "no");
    }
    else {
        printf("File %s removed\n", content);
        sendpacket(newsocketfd, 'R', "ok");
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
        }
        sendpacket(newsocketfd, 'L', file_list);
        closedir(d);
    }
    free(file_list);
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
        while (fread(piece, sizeof(char), 1024, openFile) != NULL) {
            sendpacket(newsocketfd, 'D', piece);
            memset(piece, 0, 1024);
        }
        sendpacket(newsocketfd, 'Y', "end");
        fclose(openFile);
        free(piece);
    }
    free(command);
}

void zipFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 9);
    memset(command, 0, strlen(content) + 9);
    strcpy(command, "bzip2 -z ");
    strcat(command, content);
    if (system(command) != 0) {
        printf("Compress File Error!");
        sendpacket(newsocketfd, 'Z', "no");
    }
    else {
        sendpacket(newsocketfd, 'Z', "ok");
    }
    free(command);
}

void unzipFile(char* content) {
    char* command = (char*)malloc(strlen(content) + 9);
    memset(command, 0, strlen(content) + 9);
    strcpy(command, "bzip2 -d ");
    strcat(command, content);
    if (system(command) != 0) {
        printf("Decompress File Error!");
        sendpacket(newsocketfd, 'U', "no");
    }
    else {
        sendpacket(newsocketfd, 'U', "ok");
    }
    free(command);
}

void encryptFile(char* content) {
    char* filename = (char*)malloc(sizeof(content));
    memcpy(filename, content, sizeof(content));
    char* command = (char*)malloc(strlen(filename) + 100);
    memset(command, 0, strlen(filename) + 100);
    strcpy(command, "echo ");
    printf("The file name is %s\n", filename);

    //passphrase put in
    readpacket(newsocketfd);
    strcat(command, buffer.content);

    printf("The file name is %s\n", filename);
    strcat(command, " | gpg --yes --passphrase-fd 0 -c ");
    strcat(command, filename);

    printf("The command is %s\n", command);
    if (system(command) != 0) {
        printf("Encrypt File Error!");
        sendpacket(newsocketfd, 'U', "Encrypt File Error!");
    }
    else {
        sendpacket(newsocketfd, 'U', "Encrypt File succcessful!");
    }
    free(command);
    free(filename);
}

void decryptFile(char* content) {
    char* filename = (char*)malloc(sizeof(content));
    memcpy(filename, content, sizeof(content));
    char* command = (char*)malloc(strlen(content) + 100);
    memset(command, 0, strlen(content) + 100);
    strcpy(command, "echo ");

    //passphrase put in
    readpacket(newsocketfd);
    strcat(command, buffer.content);

    strcat(command, " | gpg --yes --passphrase-fd 0 ");
    strcat(command, filename);

    if (system(command) != 0) {
        printf("Decrypt File Error!");
        sendpacket(newsocketfd, 'U', "Decrypt File Error!");
    }
    else {
        sendpacket(newsocketfd, 'U', "Decrypt File succcessful!");
    }
    free(command);
    free(filename);
}


void printError(char* message) {
    printf("%s\n", message);
    exit(1);
}
