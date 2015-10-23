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

#define PORT_NO 5678    //define port NO.

int newsocketfd;    //define socket file descriptor here

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

    // construct socket and bind socket
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    int binding = bind (socketfd, (struct sockaddr*) &serv_addr,(socklen_t) sizeof(serv_addr));
    if (binding == BIND_ERROR) {    //if bind is error
        printError("Socket binding error!");
    }
    else {
        listen (socketfd, 1);   //listen to socket
    }

    // get clilen and start to accept connection
    socklen_t clilen = sizeof(cli_addr);
    newsocketfd = accept(socketfd, (struct sockaddr*) &cli_addr,&clilen);
    if (newsocketfd == ACCEPT_ERROR) {  //if accept is error
        printError("Socket accept error!");
    }

    // read socket repeatedly
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
                case 'Z':   //check to zip or unzip
                    if (strcmp(buffer.content, "zip") == 0) {
                        readpacket(newsocketfd);
                        zipFile(buffer.content);
                    }
                    else if (strcmp(buffer.content, "unzip") == 0) {
                        readpacket(newsocketfd);
                        unzipFile(buffer.content);
                    }
                    break;
                case 'S':   //check to encrypt or decrypt
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

// to create file
void createFile(char* content) {
    //let the create file command ready
    char* command = (char*)malloc(strlen(content) + 8);
    memset(command, 0, sizeof(strlen(content) + 8));
    strcat(command, "touch \"");
    strcat(command, content);
    strcat(command, "\"");

    //do the command and check if it is succcessful
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

//to edit file
void editFile(char* content) {
    //let the check file existence command ready
    char* command = (char*)malloc(strlen(content) + 10);
    memset(command, 0, sizeof(strlen(content) + 10));
    strcat(command, "test -e \"");
    strcat(command, content);
    strcat(command, "\"");

    //do the command and check if it is succcessful
    if (system(command) != 0) {
        printf("File not exist!\n");
        sendpacket(newsocketfd, 'E', "no");
    }
    else {  //if file exist
        printf("File %s found\n", content);

        //open the file
        FILE* openFile;
        openFile = fopen(content, "w");

        //tell client that file exist
        sendpacket(newsocketfd, 'E', "ok");

        //start to get content from client repeatedly until exit message received
        while (1) {
            if (readpacket(newsocketfd) == 0) {
                if (strcmp(buffer.content, "::exit::") == 0)    //exit message received
                    break;
                else
                    fputs(buffer.content, openFile);    //put received message into file
            }
        }
        fclose(openFile);
    }
    free(command);
}

// to remove file
void removeFile(char* content) {
    //let the remove command ready
    char* command = (char*)malloc(strlen(content) + 5);
    memset(command, 0, sizeof(strlen(content) + 5));
    strcat(command, "rm \"");
    strcat(command, content);
    strcat(command, "\"");

    //do the command and check if it is succcessful
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

// to list file
void listFile() {
    // open directory
    DIR* d;
    struct dirent* dir;
    d = opendir(".");

    //make a string to save file_list
    char* file_list = (char*)malloc(1024);
    memset(file_list, 0, 1024);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            strncat(file_list, dir->d_name, sizeof(dir->d_name));
            strncat(file_list, "\n", 1);
        }
        sendpacket(newsocketfd, 'L', file_list);    //send file_list to client
        closedir(d);
    }
    free(file_list);
}

// to send file to client
void downloadFile(char* content) {
    //let the check file existence command ready
    char* command = (char*)malloc(strlen(content) + 10);
    memset(command, 0, sizeof(strlen(content) + 10));
    strcat(command, "test -e \"");
    strcat(command, content);
    strcat(command, "\"");

    //do the command to check if file exist
    if (system(command) != 0) {
        printf("File not exist!\n");
        sendpacket(newsocketfd, 'N', "no");
    }
    else {
        printf("File %s found\n", content);

        //open the file
        FILE* openFile;
        openFile = fopen(content, "r");

        // tell client file exist
        sendpacket(newsocketfd, 'Y', "ok");

        char* piece = (char*)malloc(1024);
        memset(piece, 0, 1024);

        //read content repeatedly and send to client
        while (fread(piece, sizeof(char), 1024, openFile) != NULL) {
            sendpacket(newsocketfd, 'D', piece);
            memset(piece, 0, 1024);
        }

        //send a packet to inform client that transmission is completed
        sendpacket(newsocketfd, 'Y', "end");

        fclose(openFile);
        free(piece);
    }
    free(command);
}

// to zip file
void zipFile(char* content) {
    //let the zip command ready
    char* command = (char*)malloc(strlen(content) + 11);
    memset(command, 0, strlen(content) + 11);
    strcpy(command, "bzip2 -z \"");
    strcat(command, content);
    strcat(command, "\"");

    //do the command and check if it is succcessful
    if (system(command) != 0) {
        printf("Compress File Error!");
        sendpacket(newsocketfd, 'Z', "no");
    }
    else {
        sendpacket(newsocketfd, 'Z', "ok");
    }
    free(command);
}

// to unzip file
void unzipFile(char* content) {
    //let the unzip command ready
    char* command = (char*)malloc(strlen(content) + 11);
    memset(command, 0, strlen(content) + 11);
    strcpy(command, "bzip2 -d \"");
    strcat(command, content);
    strcat(command, "\"");

    //do the command and check if it is succcessful
    if (system(command) != 0) {
        printf("Decompress File Error!");
        sendpacket(newsocketfd, 'U', "no");
    }
    else {
        sendpacket(newsocketfd, 'U', "ok");
    }
    free(command);
}

// to encrypt file
void encryptFile(char* content) {
    //start to build the command
    char* filename = (char*)malloc(sizeof(content));
    memcpy(filename, content, sizeof(content));
    char* command = (char*)malloc(strlen(filename) + 100);
    memset(command, 0, strlen(filename) + 100);
    strcpy(command, "echo \"");
    printf("The file name is %s\n", filename);

    //get passphrase
    readpacket(newsocketfd);
    strcat(command, buffer.content);
    strcat(command, "\"");

    printf("The file name is %s\n", filename);
    strcat(command, " | gpg --yes --passphrase-fd 0 -c \"");
    strcat(command, filename);
    strcat(command, "\"");

    //do the command and check if it is succcessful
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

//to decrypt file
void decryptFile(char* content) {
    //start to build the command
    char* filename = (char*)malloc(sizeof(content));
    memcpy(filename, content, sizeof(content));
    char* command = (char*)malloc(strlen(content) + 100);
    memset(command, 0, strlen(content) + 100);
    strcpy(command, "echo \"");

    //get passphrase
    readpacket(newsocketfd);
    strcat(command, buffer.content);
    strcat(command, "\"");

    strcat(command, " | gpg --yes --passphrase-fd 0 \"");
    strcat(command, filename);
    strcat(command, "\"");

    //do the command and check if it is succcessful
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

// to print error message and exit
void printError(char* message) {
    printf("%s\n", message);
    exit(1);
}
