#ifndef INCLUDE_FD_MANAGE_H
#define INCLUDE_FD_MANAGE_H

#include "general.h"
#include <vector>
#include <array>
#include "string.h"
#include "fileManager.h"
#include "stdio.h"
#define FD_NONE 0 // this fd haven't specify its type
#define FD_DATA 1
#define FD_CMD 2
#define FD_LISTEN 3

#define PHASE_NONE 0
#define PHASE_DOING 1
#define PHASE_END 2

using std::vector;
using std::array;
using std::max;

struct fdEntry{
    int fd;
    bool used;
    int type; // FD_DATA or FD_CMD
    int phase; // PHASE_NONE , PHASE_DOING , PHASE_END
    char name[MAX_NAME_SIZE];
    char* buf;
    char *pBase,*pFront,*pEnd;
    size_t size;
    // function
    fdEntry();
    fdEntry(int fd,int type);
    ~fdEntry();
    void clear();
    void remove();
    void responseLogin();
    void setData(const char* vBuf,size_t vSize);
    size_t flushWrite();
};

fdEntry::fdEntry()
:fd(-1),used(false),type(FD_NONE){
    clear();    
}

fdEntry::fdEntry(int fd,int type)
:fd(fd),used(true),type(type)
{
    clear();   
}

fdEntry::~fdEntry(){
    delete[] buf;
}

void fdEntry::clear(){
    buf = new char[MAX_BUF_SIZE];
    pEnd = pFront = pBase = buf;
    phase = PHASE_NONE;
}

void fdEntry::remove(){
    fd = -1;
    used = false;
    clear();
}

void fdEntry::responseLogin(){
    cmdHeader header;
    header.cmdType = CMD_LIST;
    header.setName(name);
    sendObject(fd,&header,sizeof(header));
    printf("Login Response Ready\n");
} 

void fdEntry::setData(const char* vBuf,size_t vSize){
    // append new data to current data
    char* oldbuf = buf;
    size_t old_size = pEnd - pBase;
    buf = new char[old_size + vSize];
    memcpy(buf,pBase,old_size);
    memcpy(buf+old_size,vBuf,vSize);
    pBase = pFront = buf;
    pEnd = pBase + vSize + old_size;
    delete[] oldbuf;
}

size_t fdEntry::flushWrite(){
    if(pBase < pEnd){
        // writable, write some
        size_t nW = write(fd,pBase,pEnd - pBase);
        if(nW < 0){
            if(errno != EWOULDBLOCK){
                fprintf(stderr,"Error on Write FD %d\n",fd);
                exit(1);
            }
        }
        pBase += nW;
    }
}

struct FDManager{
    // functions
    FDManager();
    void addFD(int fd,int type); 
    // data
    int maxfdp1;
    array<fdEntry,MAX_FD> data;
    int size;
};

FDManager::FDManager(){
    maxfdp1 = 0;
    size = 0;
}

void FDManager::addFD(int fd,int type = FD_NONE){
    // search all
    bool found = false;
    for(int i=0;i<size;i++){
        auto& entry = data[i];
        if(!entry.used){
            entry.fd = fd;
            entry.used = true;
            found = true;
            break;
        }     
    }
    // not space, add one
    if(!found){
        data[size++] = fdEntry(fd,type);
        maxfdp1 = max(maxfdp1,fd+1);
    }
}


#endif
