#ifndef INCLUDE_FILE_MANAGER_H
#define INCLUDE_FILE_MANAGER_H
#include "fileInfo.h"
#include "vector"
#include <iostream>
#include <fstream>
#include <sstream>
#include "cstring"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sys/stat.h"


#define DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP)

using std::stringstream;
using std::ofstream;
using std::vector;
using std::strcmp;
using std::string;

struct userEntry{
    char name[MAX_NAME_SIZE];
    vector<fileMeta> files;
    string dir_path;
    // function
    userEntry(const char* vName);
    void createWelcomeFile(int id = 0);
    transferHeader generateListHeader();
    string filePath(const char* filename);
};

userEntry::userEntry(const char* vName){
    strcpy(name,vName);
    dir_path = "server_storage/";
    dir_path += name;
    dir_path += "/";
}
string userEntry::filePath(const char* filename){
    return dir_path + filename;
}

void userEntry::createWelcomeFile(int id ){
    char buf[MAX_BUF_SIZE];
    bzero(buf,MAX_BUF_SIZE);
    snprintf(buf,MAX_BUF_SIZE,"Welcome Seq_%d",id);
    string filename(buf);
    ofstream fout(dir_path + filename);
    string content("Welcome Message for ");
    for(int i=0;i<id*1000;i++){
        content += name;    
    }
    fileMeta meta;
    strcpy(meta.name,filename.c_str());
    size_t fileSize = 0;
    size_t nWrite;
    fout.write(content.c_str(),content.size());
    meta.size = content.size();
    files.push_back(meta);
}

transferHeader userEntry::generateListHeader(){
    transferHeader tH;
    tH.type = TRANSFER_LIST;
    tH.tCount = files.size();
    return tH;
}

struct fileManager{
    // function
    fileManager();
    bool checkUserExist(const char* name);
    void addUser(const char* name);
    userEntry& getUserEntry(const char* name);
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
        userEntry ent(name);
        char path[MAX_BUF_SIZE];
        snprintf(path,MAX_BUF_SIZE,"server_storage/%s",name);
        mkdir(path,DIR_MODE);
        fprintf(stderr,"New storage folder '%s' created for user '%s'\n",path,name);       
        // create test file
#ifdef CREATE_TEST
        srand(time(NULL));
        int times = rand()%10;
        for(int i=0;i<times;i++){
            ent.createWelcomeFile(i);
        }
#endif
        // move data
        data.push_back(std::move(ent));
    }    
}

userEntry& fileManager::getUserEntry(const char* name){
    for(auto& entry : data){
        if(strcmp(entry.name,name)==0){
            return entry;
        }
    }
}



#endif
