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

#define PORT_NO 5678    //define port NO.

void printError(char* message);     //Error Message print
void printMenu();       //print the user menu
void choiceRouter();    //route user's choice

int socketfd;   //define socket file descriptor here

void main(int args, char* argv[]) {
    struct sockaddr_in serv_addr;       //define server address
    struct hostent* server_info;        //save server_info after gethostbyname()
    packet = (unsigned char*)malloc(sizeof(buffer));    //

    socketfd = socket(AF_INET, SOCK_STREAM, 0);     //build socket and assign it ot socketfd
    if (&socketfd == NULL) {
        printError("Socket build error!\n");
    }

    // analyze the hostname to find address
    server_info = gethostbyname(argv[1]);   //get host's address info and assign it to server_info
    if (server_info == NULL) {
        printError("Host not found!\n");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));   //cleanup serv_addr first

    //assign server info into serv_addr
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server_info->h_addr, sizeof(server_info->h_addr));
    serv_addr.sin_port = htons(PORT_NO);

    //connect to server
    if (connect(socketfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) == CONNECT_ERROR) {
        printError("Host not found!\n");
    }

    //print menu repeatedly
    while (1) {
        printMenu();
    }
}

// to create file on server
void createFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("\n");
    gets(fileName);     //get user's filename

    sendpacket(socketfd, 'C', fileName);    //tell server the fileName to create

    //receive server message to know whether file be created or not
    readpacket(socketfd);
    if (strcmp(buffer.content, "ok") == 0)
        printf("File %s created succcessful!\n", fileName);
    else if (strcmp(buffer.content, "no") == 0)
        printf("File %s created failed!\n", fileName);
}

// to edit file on server
void editFile() {
    char fileName[256];
    printf("Please input fileName: ");
    scanf("\n");
    gets(fileName);     //get fileName

    sendpacket(socketfd, 'E', fileName);    //tell server filename to check existence

    //receive message from server about file existence
    readpacket(socketfd);
    if (strcmp(buffer.content, "no") == 0)
        printf("File not existed!\n");
    else if (strcmp(buffer.content, "ok") == 0) {   //if file exist
        printf("Please input the content for %s below:(::exit:: to exit)\n", fileName);
        printf("============================================================\n");
        char edit_Content [1024];

        //get user input repeatedly until "::exit::" is detected
        while (fgets(edit_Content, 1024, stdin)) {
            if (strcmp(edit_Content, "::exit::\n") == 0) {
                sendpacket(socketfd, 'E', "::exit::");
                break;
            }
            sendpacket(socketfd, 'E', edit_Content);
            memset(edit_Content, 0, 1024);  //cleanup edit_Content
        }
    }
}

// to delete file on server
void removeFile(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("\n");
    gets(fileName);     //get fileName


    sendpacket(socketfd, 'R', fileName);   //tell server filename to check existence

    //receive server message to know whether file be deleted or not
    readpacket(socketfd);
    if (strcmp(buffer.content, "ok") == 0)
        printf("File %s removed succcessful!\n", fileName);
    else if (strcmp(buffer.content, "no") == 0)
        printf("File %s removed failed!\n", fileName);
}

//to list all stuff in this directory
void listFile(){
    sendpacket(socketfd, 'L', "123");   //tell server to list file
    readpacket(socketfd);   //receive the list
    printf("%s",buffer.content);    //print the list
}

// to download file from server
void download(){
    char fileName[256];
    printf("Please input fileName: ");
    scanf("\n");
    gets(fileName);     //get fileName


    sendpacket(socketfd, 'D', fileName);    //tell server filename to check existence

    //receive server message to know whether file exist or not
    readpacket(socketfd);
    if (buffer.command == 'N')
        printf("File %s not found\n", fileName);
    else if (buffer.command == 'Y') {       //if file exist
        system("mkdir received");   //make a directory
        printf("File %s found, Downloading...\n", fileName);

        // assign the file_path to be inside the directory
        char* file_path = (char*)malloc(sizeof(fileName) + 11);
        memset(file_path, 0, sizeof(fileName) + 11);
        strcpy(file_path, "./received/");
        strcat(file_path, fileName);

        //open the file
        FILE* writeFile;
        writeFile = fopen(file_path, "w");

        //receive content repeatedly from server
        while(1) {
            readpacket(socketfd);
            if (buffer.command == 'D')  //write content into the file
                fwrite(buffer.content, sizeof(char), 1024, writeFile);
            else if (buffer.command == 'Y') {   //this means all content transferred completed
                printf("Download Completed!\n");
                printf("File is stored at: %s\n", file_path);
                break;
            }
        }
        fclose(writeFile);      //close the file
        free(file_path);        //clean the memory
    }
}

// to zip the file
void zipFile() {
    char choice[50];
    char fileName[256];

    //ask user's choice
    printf("Do you want (Z)ip or (U)nzip file?\n");
    printf("Please input Z or U: ");
    scanf("\n");
    gets(choice);

    //check user's choice and tell server
    if (choice[0] == 'Z')
        sendpacket(socketfd, 'Z', "zip");
    else if (choice[0] == 'U')
        sendpacket(socketfd, 'Z', "unzip");
    else {
        printf("Input is incorrect!\n");    //Input is not correct
        return;
    }

    //ask fileName
    printf("Please input your file name: ");
    scanf("\n");
    gets(fileName);

    sendpacket(socketfd, 'Z', fileName);    //tell server the filename
    readpacket(socketfd);   //receive result from server
    printf("%s\n", buffer.content);
}

void secureFile() {
    char choice[50];
    char fileName[256] = {0};
    char password[100] = {0};

    //ask user's choice
    printf("Do you want (E)ncrypt or (D)ecrypt file?\n");
    printf("Please input E or D: ");
    scanf("\n");
    gets(choice);

    //check user's choice and tell server
    if (choice[0] == 'E')
        sendpacket(socketfd, 'S', "encrypt");
    else if (choice[0] == 'D')
        sendpacket(socketfd, 'S', "decrypt");
    else {
        printf("Input is incorrect!\n");    //Input is not correct
        return;
    }

    //ask filename and tell server
    printf("Please input your file name: ");
    scanf("\n");
    gets(fileName);
    sendpacket(socketfd, 'S', fileName);

    //ask user's passphrase and tell server
    printf("Please input your passphrase: ");
    scanf("%s", password);
    sendpacket(socketfd, 'P', password);

    //receive result from server
    readpacket(socketfd);
    printf("%s\n", buffer.content);
}

//Error Message print
void printError(char* message) {
    printf("%s\n", message);
    exit(1);
}

//to print the user menu
void printMenu() {
    printf("Press [Enter] key to Continue...\n");
    char a[1024];
    fgets(a, 1024, stdin);
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
    printf("%s\n", "(S)ecurity");
    printf("%s\n", "(B)ye Bye!");
    printf("%s\n", "################################################");
    printf("%s", "Which do you want to choose? (C, E, R, L, D, Z, B): ");
    choiceRouter();
}

void choiceRouter() {
    //TODO:
    char choice[50];
    scanf("%s",choice);     //get user's choice
    system("clear");

    switch (choice[0]) {
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
        case 'S': case 's':
            printf("%s\n", "#####################");
            printf("     Encrypt/Decrypt File     ");
            printf("\n%s\n", "#####################");
            secureFile();
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
