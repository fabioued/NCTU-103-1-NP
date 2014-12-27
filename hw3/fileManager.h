#ifndef INCLUDE_FILE_MANAGER_H
#define INCLUDE_FILE_MANAGER_H
#include "fileInfo.h"
#include "vector"
#include <iostream>
#include "cstring"
#include <stdio.h>
#include "sys/stat.h"

#define DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP)

using std::vector;
using std::strcmp;

struct userEntry{
    char name[MAX_NAME_SIZE];
    vector<fileInfo*> files;
};

struct fileManager{
    // function
    fileManager();
    bool checkUserExist(const char* name);
    void addUser(const char* name);

    // data
    vector<userEntry> data; 
};

fileManager::fileManager(){
    mkdir("server_storage",DIR_MODE);
}

bool fileManager::checkUserExist(const char* name){
    for(auto& entry:data){
        if(strcmp(name,entry.name)==0){
            return true;
        }    
    }
    return false;
}

void fileManager::addUser(const char* name){
    if(!checkUserExist(name)){
        userEntry ent;
        strcpy(ent.name,name);
        data.push_back(std::move(ent));
        char path[MAX_BUF_SIZE];
        snprintf(path,MAX_BUF_SIZE,"server_storage/%s",name);
        mkdir(path,DIR_MODE);
        fprintf(stderr,"New storage folder '%s' created for user '%s'\n",path,name);       
    }    
}


#endif
