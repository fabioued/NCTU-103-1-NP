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
    char* rbuf;
    char *rBase,*rFront,*rEnd;
    size_t size;
    bool writeEnd;
    fileMeta meta; // for transfer use
    size_t total;
    bool readEnd;
    // function
    string filename; // for fileupload
    fdEntry();
    fdEntry(int fd,int type);
    fdEntry(const fdEntry& rhs) = delete;
    fdEntry& operator=(const fdEntry& rhs) = delete;
    ~fdEntry();
    void clear();
    void remove();
    void setData(const void*const vBuf,size_t vSize);
    void setReadHeader();
    bool flushWrite();
    bool patchRead(const void* buf,size_t vSize);
    void setReadFile(const fileMeta& meta,const string& fullPath);
    void writeBufferAsFile();
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
    delete[] rbuf;
}

void fdEntry::clear(){
    buf = new char[MAX_BUF_SIZE];
    rbuf = new char[MAX_BUF_SIZE];
    pEnd = pFront = pBase = buf;
    rEnd = rFront = rBase = rbuf;
    phase = PHASE_NONE;
    type = FD_NONE;
    writeEnd = false;
    readEnd = false;
    filename = "";
}

void fdEntry::remove(){
    printf("FD %d remove\n",fd);
    close(fd);
    fd = -1;
    used = false;
    clear();
}



void fdEntry::setData(const void* const vBuf ,size_t vSize){
    fprintf(stderr,"Remaining size : %d , Appending data : %u\n",pEnd-pBase,vSize);
    // append new data to current data
    char* oldbuf = buf;
    size_t old_size = pEnd - pBase;
    buf = new char[old_size + vSize];
    memcpy(buf,pBase,old_size);
    memcpy(buf+old_size,vBuf,vSize);
    pBase = pFront = buf;
    pEnd = pBase + vSize + old_size;
    writeEnd = false;
    total = 0;
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
        total += nW;
        pBase += nW;
        fprintf(stderr,"Write %u bytes , remain %d bytes\n",nW,pEnd - pBase);
    }
    else{
        if(!writeEnd){
            fprintf(stderr,"Write buffer empty.\n");
        }
        if (total > 0){
            fprintf(stderr,"Total written  : %u\n",total);
            total = 0;
        }
        writeEnd = true;
    }
    return !writeEnd;
}



void fdEntry::setReadFile(const fileMeta& meta,const string& fullPath){
    delete[] rbuf;
    rbuf = new char[meta.size];
    fprintf(stderr,"[Meta] File %s,  size : %u\n",meta.name,meta.size);
    rBase = rbuf;
    rEnd = rbuf + meta.size; // point to final bytes position
    fprintf(stderr,"Mem to read:%d\n",rEnd - rBase);
    filename = fullPath; // filename to store
}

void fdEntry::setReadHeader(){
    delete[] rbuf;
    rbuf = new char[sizeof(cmdHeader)];
    rBase = rbuf;
    rEnd = rbuf + sizeof(cmdHeader);
}

void fdEntry::writeBufferAsFile(){
    fprintf(stderr,"File (%d bytes)Read OK!\n",rEnd - rbuf);
    fileInfo::writeAsFile(filename,rbuf,rEnd - rbuf);
}

bool fdEntry::patchRead(const void* vBuf,size_t vSize){
    fprintf(stderr,"Patch : %u , remain : %d\n",vSize,rEnd - rBase);
    memcpy(rBase,vBuf,vSize);
    rBase += vSize;
    if(rBase >= rEnd){
        return false;
    }
    else{
        return true;
    }
}

struct FDManager{
    // functions
    FDManager();
    fdEntry& addFD(int fd,int type); 
    // data
    int maxfdp1;
    array<fdEntry,MAX_FD> data;
    int size;
};

FDManager::FDManager(){
    maxfdp1 = 0;
    size = 0;
}

fdEntry& FDManager::addFD(int fd,int type = FD_NONE){
    // search all
    fdEntry* pE;
    bool found = false;
    for(int i=0;i<size;i++){
        auto& entry = data[i];
        if(!entry.used){
            pE = &entry;
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
        pE = &data[size];
        ++size;
    }
    maxfdp1 = max(maxfdp1,fd+1);
    return *pE;
}
#endif
