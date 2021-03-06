#ifndef INCLUDE_GENERAL_H
#define INCLUDE_GENERAL_H

#include "stdio.h"

//#define CREATE_TEST
#define DELAY_MS 0
#define WRITE_TO_LOG
//#define HEADER_DUMMY

#define FD_STDIN 0

#define MAX_BUF_SIZE 70000
#define MAX_NAME_SIZE 255
#define MAX_FD 100

#define CMD_LOGIN 1
#define CMD_LIST 2
#define CMD_PUT 3
#define CMD_GET 4

#define TRANSFER_LIST 1
#define TRANSFER_GET 2


#define FD_NONE 0 // this fd haven't specify its type
#define FD_CMD 1
#define FD_LISTEN 2
#define FD_GET 3
#define FD_PUT 4

#define PHASE_NONE 0
#define PHASE_DOING 1
#define PHASE_END 2




//// FILE META
struct fileMeta{
    char name[MAX_NAME_SIZE];
    size_t size;
    bool operator==(const fileMeta& rhs);
};

bool fileMeta::operator==(const fileMeta& rhs){
    return strcmp(name,rhs.name)==0;
}

//// CMD HEADER

struct cmdHeader{
    char name[MAX_NAME_SIZE];
    int cmdType;
#ifdef HEADER_DUMMY
    char dum[65536];
#endif
    fileMeta meta;
    void setName(const char* vName);
};

void cmdHeader::setName(const char* vName){
    strncpy(name,vName,MAX_NAME_SIZE);
    name[MAX_NAME_SIZE-1] = '\0';
}


//// TRANSFER HEADER
struct transferHeader{
    int type; // TRANSFER_*
    union {
        size_t tCount;
        fileMeta tMeta;
    } transData;
};
#define tCount transData.tCount
#define tMeta transData.tMeta


void setNonBlock(int fd){
    // socket option : non-blocking
    int flag=fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}

void sendObject(int fd,void* obj,size_t size){
    write(fd,obj,size);
}

template<typename T>
void sendObject(int fd,T& obj){
    write(fd,&obj,sizeof(obj));
}

void readObject(int fd,void* Pobj,size_t size){
    char* obj = (char*)Pobj;
    char buf[MAX_BUF_SIZE];
    size_t offset = 0;
    int read_n;
    while(size > 0){
        read_n = read(fd,buf,size > MAX_BUF_SIZE ? MAX_BUF_SIZE : size );
        if(read_n <= 0){
            return;
        }
        fprintf(stderr,"Object Read:%d\n",read_n);
        size -= read_n;
        memcpy(obj+offset,buf,read_n);
        offset += read_n;
    }
}

template<typename T>
bool readObject(int fd,T& rObj){
    char* obj = (char*)&rObj;
    size_t size =  sizeof(rObj); 
    char buf[MAX_BUF_SIZE];
    int read_n;
    size_t offset = 0;
    while(size > 0){
        read_n = read(fd,buf,size > MAX_BUF_SIZE ? MAX_BUF_SIZE : size );
        if(read_n <= 0){
            return false;
        }
        fprintf(stderr,"Object Read:%d\n",read_n);
        size -= read_n;
        memcpy(obj+offset,buf,read_n);
        offset += read_n;
    }
    return true;
}

#endif
