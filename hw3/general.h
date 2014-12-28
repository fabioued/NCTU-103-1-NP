#ifndef INCLUDE_GENERAL_H
#define INCLUDE_GENERAL_H

#include "stdio.h"

#define MAX_BUF_SIZE 2048
#define MAX_NAME_SIZE 255
#define MAX_FD 100

#define CMD_LOGIN 1
#define CMD_TRANSFER 2
#define CMD_LIST 3

struct cmdHeader{
    char name[MAX_NAME_SIZE];
    int cmdType;
    void setName(const char* vName);
};

void cmdHeader::setName(const char* vName){
    strncpy(name,vName,MAX_NAME_SIZE);
    name[MAX_NAME_SIZE-1] = '\0';
}

void setNonBlock(int fd){
    // socket option : non-blocking
    int flag=fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}

void sendObject(int fd,void* obj,size_t size){
    write(fd,obj,size);
}

void readObject(int fd,void* obj,size_t size){
    char buf[MAX_BUF_SIZE];
    int read_n = read(fd,buf,size);
    printf("Object Read:%d\n",read_n);
    if(read_n == size){
        memcpy(obj,buf,size);        
    }
}


#endif
