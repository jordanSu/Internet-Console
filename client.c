#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>     //socket()
#include <netdb.h>  //struct hostent
#include <netinet/in.h>  //struct sockaddr_in
#include <unistd.h>
#include "packet.h"

#define CONNECT_ERROR -1

#define PORT_NO 5678

void printError(char* message);
void printMenu();       //print the user menu
void choiceRouter();    //route user's choice

int socketfd;

int main(int args, char* argv[]) {
    struct sockaddr_in serv_addr;
    struct hostent* server_info;
    packet = (unsigned char*)malloc(sizeof(buffer));

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (&socketfd == NULL) {
        printError("Socket build error!");
    }

    // analyze the hostname to find address
    server_info = gethostbyname(argv[1]);
    if (server_info == NULL) {
        printError("Host not found!");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server_info->h_addr, sizeof(server_info->h_addr));
    //printf ("this is address:%d",serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(PORT_NO);

    if (connect(socketfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) == CONNECT_ERROR) {
        printError("Host not found!");
    }
    while (1) {
        printMenu();
    }
    close(socketfd);
    return 0;
}

void createFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("%s",fileName);
    printf("Your fileName is: %s\n",fileName);
    sendpacket(socketfd, 'C', fileName);
    /*
    memset(&buffer, 0, sizeof(buffer));
    buffer.command = 'C';
    strncpy(buffer.content, fileName, sizeof(fileName));
    memcpy(packet, &buffer, sizeof(buffer));
    write(socketfd, packet, sizeof(buffer));
    */
}

void editFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("%s",fileName);
    printf("Your fileName is: %s\n",fileName);
    sendpacket(socketfd, 'E', fileName);
    readpacket(socketfd);
    if (strcmp(buffer.content, "no") == 0)
        printf("File not existed!\n");
    else {
        printf("Please input the content below:\n");
        printf("===============================\n");
        //TODO: get user input for edit file
        char edit_Content [1024];
        scanf("%s", edit_Content);
        sendpacket(socketfd, 'E', edit_Content);


    }
}

void removeFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("%s",fileName);
    printf("Your fileName is: %s\n",fileName);
    sendpacket(socketfd, 'R', fileName);
}

void listFile(){

}

void download(){

}

void printError(char* message) {
    printf("%s\n", message);
    exit(1);
}

void printMenu() {
    //TODO:
    system("read -p \"Press [Enter] key to Continue...\"");
    system("clear");
    printf("%s\n", "################################################");
    printf("%s\n", "           Welcome to Internet editor           ");
    printf("%s\n", "################################################");
    printf("%s\n", "   There are some option you can choose below   ");
    printf("%s\n", "################################################");
    printf("%s\n", "(C)reate");
    printf("%s\n", "(E)dit");
    printf("%s\n", "(R)emove");
    printf("%s\n", "(L)ist");
    printf("%s\n", "(D)ownload");
    printf("%s\n", "################################################");
    printf("%s", "Which do you want to choose? (C, E, R, L, D): ");
    choiceRouter();
}

void choiceRouter() {
    //TODO:
    char choice;
    scanf("\n%c",&choice);
    system("clear");
    switch (choice) {
        case 'C':
        case 'c':
            createFile();
            break;
        case 'E':
        case 'e':
            editFile();
            break;
        case 'R':
        case 'r':
            removeFile();
            break;
        case 'L':
        case 'l':
            listFile();
            break;
        case 'D':
        case 'd':
            download();
            break;
        default:
            printf("Selection not found\n");
    }
}
