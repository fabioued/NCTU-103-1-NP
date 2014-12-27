#ifndef INCLUDE_FILE_INFO_H
#define INCLUDE_FILE_INFO_H
#include "general.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#define INIT_FILE_SIZE 1024
using std::string;
using std::ifstream;
using std::memcpy;
using std::cout;
using std::endl;

struct fileInfo{
    // functions
    fileInfo();
    ~fileInfo();
    void load(const string& v_filename);
    // internal use
    void storeBlock(const char* block,size_t bufSize);
    void expand();
    // data
    bool loaded;
    char *data;
    size_t size;
    size_t capacity;
    string filename;
};

fileInfo::fileInfo(){
    loaded = false;
    size = 0;
    capacity = INIT_FILE_SIZE;
    data = new char[capacity];
}

fileInfo::~fileInfo(){
    delete[] data;
}

void fileInfo::load(const string& v_filename){
    loaded = true;
    filename = v_filename;
    char buf[MAX_BUF_SIZE];
    ifstream file(v_filename);
    size_t len;
    do{
        file.read(buf,MAX_BUF_SIZE);
        len = file.gcount();
        cout << "Read block : " << len << endl;
        storeBlock(buf,len);
        
    }while(file);
}

void fileInfo::storeBlock(const char* buf,size_t bufSize){
    while(size + bufSize > capacity){
        expand();
    }
    memcpy(data+size,buf,bufSize);
    size += bufSize;
}

void fileInfo::expand(){
    char* old = data;
    capacity = capacity * 2;
    data = new char[capacity];
    // copy
    memcpy(data,old,sizeof(char)*size);
    delete[] old;
}







#endif
