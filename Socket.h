#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <unistd.h>
#include <cstring>
#include <list>
#include <arpa/inet.h>

class Socket{
    public:
    int sock;
    char buffer[512];
    std::string nowRead;
    int messageSize = -1;
    std::list<std::string> message;

    void readData() {
    std::memset(buffer, 0, 512);
    int read_size = read(sock, buffer, sizeof(buffer));

    if (read_size < 1) {
        fprintf(stderr, "%d disconnected\n", sock);
        throw "client disconnected";
    }
    nowRead += std::string(buffer, read_size);

    while (true) {
        if (messageSize < 0) {
            if (nowRead.size() >= sizeof(int)) {
                messageSize = ntohl(*(int*)nowRead.c_str());
                nowRead = nowRead.substr(sizeof(int));
            } else {
                break;
            }
        }

        if (messageSize > 0 && nowRead.length() >= messageSize) {
            std::string fullMessage = nowRead.substr(0, messageSize);
            message.push_back(fullMessage);
            nowRead = nowRead.substr(messageSize);
            messageSize = -1;
        } else {
            break;
        }
    }
}

    void writeData(std::string message){
        int size = htonl(message.size());
        if(write(sock, (char*)&size, sizeof(int)) < 0){
            printf("%s\n", "write data");
        }
        if(write(sock, message.c_str(), message.size()) < 0){
            printf("%s\n", "write data");
        }
    }

    void closeSocket(){
        close(sock);
        printf("socket closed\n");
    }
};


#endif 