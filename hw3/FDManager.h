#ifndef INCLUDE_FD_MANAGE_H
#define INCLUDE_FD_MANAGE_H

#include "general.h"
#include <vector>
#include <array>
#include "string.h"
#include "fileManager.h"
#include "stdio.h"

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
    bool writeEnd;
    // function
    fdEntry();
    fdEntry(int fd,int type);
    fdEntry(const fdEntry& rhs) = delete;
    fdEntry& operator=(const fdEntry& rhs) = delete;
    ~fdEntry();
    void clear();
    void remove();
    void setData(const void*const vBuf,size_t vSize);
    bool flushWrite();
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
    writeEnd = false;
}

void fdEntry::remove(){
    fd = -1;
    used = false;
    clear();
}



void fdEntry::setData(const void* const vBuf ,size_t vSize){
    printf("Appending data : %u\n",vSize);
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

bool fdEntry::flushWrite(){
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
    else{
        writeEnd = true;
    }
    return writeEnd;
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
        data[size].fd = fd;
        data[size].type = type;
        data[size].used = true;
        ++size;
        maxfdp1 = max(maxfdp1,fd+1);
    }
}


#endif
