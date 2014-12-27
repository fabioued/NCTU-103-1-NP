#ifndef INCLUDE_FD_MANAGE_H
#define INCLUDE_FD_MANAGE_H

#include "general.h"
#include <vector>
#include "string.h"
#include "fileManager.h"
#define FD_NONE 0 // this fd haven't specify its type
#define FD_DATA 1
#define FD_CMD 2
#define FD_LISTEN 3

#define PHASE_NONE 0
#define PHASE_DOING 1
#define PHASE_END 2

using std::vector;
using std::max;

struct fdEntry{
    int fd;
    bool used;
    int type; // FD_DATA or FD_CMD
    int phase; // PHASE_NONE , PHASE_DOING , PHASE_END
    char name[MAX_NAME_SIZE];
    char buf[MAX_BUF_SIZE];
    char *pBase,*pFront;
    size_t size;
    // function
    fdEntry():fd(-1),used(false),type(FD_NONE){ clear();}
    fdEntry(int fd,int type):fd(fd),used(true),type(type){clear();}
    void clear();
    void remove();
    void responseLogin();
    void setData(const char* buf,size_t vSize);
};

void fdEntry::clear(){
    pFront = pBase = buf;
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
} 

void fdEntry::setData(const char* buf,size_t vSize){
    
}

struct FDManager{
    // functions
    FDManager();
    void addFD(int fd,int type); 
    // data
    int maxfdp1;
    vector<fdEntry> data;
};

FDManager::FDManager(){
    maxfdp1 = 0;
}

void FDManager::addFD(int fd,int type = FD_NONE){
    // search all
    bool found = false;
    for(auto& entry : data){
        if(!entry.used){
            entry.fd = fd;
            entry.used = true;
            found = true;
            break;
        }     
    }
    // not space, add one
    if(!found){
        data.push_back(fdEntry(fd,type));
        maxfdp1 = max(maxfdp1,fd+1);
    }
}


#endif
