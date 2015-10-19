#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>     //socket()
#include <netdb.h>  //struct hostent
#include <netinet/in.h>  //struct sockaddr_in
#include <unistd.h>
#include <dirent.h>
#include "packet.h"

#define CONNECT_ERROR -1

#define PORT_NO 5678

void printError(char* message);
void printMenu();       //print the user menu
void choiceRouter();    //route user's choice

int socketfd;

void main(int args, char* argv[]) {
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
}

void createFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("%s",fileName);
    sendpacket(socketfd, 'C', fileName);
    readpacket(socketfd);
    if (strcmp(buffer.content, "ok") == 0)
        printf("File %s created succcessful!", fileName);
    else if (strcmp(buffer.content, "no") == 0)
        printf("File %s created failed!", fileName);

}

void editFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("%s",fileName);
    sendpacket(socketfd, 'E', fileName);
    readpacket(socketfd);
    if (strcmp(buffer.content, "no") == 0)
        printf("File not existed!\n");
    else if (strcmp(buffer.content, "ok") == 0){
        printf("Please input the content for %s below:(1024 characters only)\n", fileName);
        printf("============================================================\n");
        char edit_Content [1024];
        scanf("\n");
        fgets(edit_Content, 1024, stdin);
        sendpacket(socketfd, 'E', edit_Content);
    }
}

void removeFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("%s",fileName);
    sendpacket(socketfd, 'R', fileName);
    readpacket(socketfd);
    if (strcmp(buffer.content, "ok"))
        printf("File %s removed succcessful!", fileName);
    else if (strcmp(buffer.content, "no"))
        printf("File %s removed failed!", fileName);
}

void listFile(){
    sendpacket(socketfd, 'L', "123");
    readpacket(socketfd);
    printf("%s",buffer.content);
}

void download(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("%s",fileName);
    printf("Your fileName is: %s\n",fileName);
    readpacket(socketfd);
    if (buffer.command == 'N')
        printf("File %s not found", fileName);
    else if (buffer.command == 'Y') {
        system("mkdir received");
        printf("File %s found, Downloading...\n", fileName);
        char* file_path = (char*)malloc(sizeof(fileName) + 11);
        memset(file_path, 0, sizeof(fileName) + 11);
        strcpy(file_path, "./received/");
        strcat(file_path, fileName);
        FILE* writeFile;
        writeFile = fopen(file_path, "w");
        while(1) {
            readpacket(socketfd);
            if (buffer.command == 'D')
                fputs(buffer.content, writeFile);
            else if (buffer.command == 'Y') {
                printf("Download Completed!\n");
                printf("File is stored at: %s", file_path);
                break;
            }
        }
        fclose(writeFile);
        free(file_path);
    }
}

void zipFile() {

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
    printf("%s\n", "(Z)ip/Unzip");
    printf("%s\n", "(B)ye Bye!");
    printf("%s\n", "################################################");
    printf("%s", "Which do you want to choose? (C, E, R, L, D, Z, B): ");
    choiceRouter();
}

void choiceRouter() {
    //TODO:
    char choice;
    scanf("\n%c",&choice);
    system("clear");
    switch (choice) {
        case 'C': case 'c':
            printf("%s\n", "#####################");
            printf("     Create File     ");
            printf("\n%s\n", "#####################");
            createFile();
            break;
        case 'E': case 'e':
            printf("%s\n", "#####################");
            printf("     Edit File     ");
            printf("\n%s\n", "#####################");
            editFile();
            break;
        case 'R': case 'r':
            printf("%s\n", "#####################");
            printf("     Remove File     ");
            printf("\n%s\n", "#####################");
            removeFile();
            break;
        case 'L': case 'l':
            printf("%s\n", "#####################");
            printf("     List File     ");
            printf("\n%s\n", "#####################");
            listFile();
            break;
        case 'D': case 'd':
            printf("%s\n", "#####################");
            printf("     Download File     ");
            printf("\n%s\n", "#####################");
            download();
            break;
        case 'Z': case 'z':
            printf("%s\n", "#####################");
            printf("     Zip/Unzip File     ");
            printf("\n%s\n", "#####################");
            zipFile();
            break;
        case 'B': case 'b':
            system("clear");
            system("echo 'Bye Bye~'");
            system("read -p \"Press [Enter] key to Continue...\"");
            system("clear");
            close(socketfd);
            exit(0);
        default:
            printf("Selection not found\n");
    }
}
