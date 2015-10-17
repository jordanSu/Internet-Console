char* packet;

struct packet_content {     //packet format
    char command;
    char content[1024];
}buffer;

int readpacket(int socketfd) {
    if (read(socketfd, packet, sizeof(buffer)) > 0) {
        memset(&buffer, 0, sizeof(buffer));
        memcpy(&buffer, packet, sizeof(buffer));
        return 0;
    }
    else
        return 1;
}

void sendpacket(int socketfd, char command, char* content) {
    memset(&buffer, 0, sizeof(buffer));
    buffer.command = command;
    strncpy(buffer.content, content, strlen(content));
    memcpy(packet, &buffer, sizeof(buffer));
    write(socketfd, packet, sizeof(buffer));
}
