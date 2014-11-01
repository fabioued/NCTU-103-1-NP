#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>
#include <sstream>
#ifndef PORT
#define PORT 50001
#endif

using namespace std;

int accu;

int ProcessCMD(const char* input,char* reply); 
const int max_buf = 2048;

int main(){

    sockaddr_in addr_self,addr_peer;
    socklen_t len_cli;

    int fd_self,fd_peer;

    fd_self = socket(AF_INET,SOCK_STREAM,0);

    if(fd_self==0){
        cout << "Socket error." << endl;
        return 1;
    }


    addr_self.sin_family = AF_INET;
    addr_self.sin_addr.s_addr = INADDR_ANY;
    addr_self.sin_port = htons(PORT);


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cout << "bind error" << endl;
        return 1;
    }

    if(listen(fd_self,10)){
       cout << "listen error" << endl; 
        return 1;
    }
    
    char buf[max_buf];
    char reply[max_buf];

    while(true){

        fd_peer = accept(fd_self,(sockaddr*)&addr_peer,&len_cli);
        if(fd_peer==-1){
            cout << "accept error" << endl;
            return 1;
        }
        accu = 0;
        cout << "Accept new connection" << endl;

        
        
        int len_recv;
        do{
            bzero(buf,max_buf);
            len_recv = read(fd_peer,buf,max_buf-1);
            cout << "Recv length:" << len_recv << endl;
            cout << "Readed:" << buf << endl;
            buf[max_buf-1] = '\0';
            int stat = ProcessCMD(buf,reply);
            if(stat == 0){
                break;
            }
            cout << "Reply Length:" << strlen(reply) << endl;
            write(fd_peer,reply,strlen(reply));
            cout << "------------------" << endl;  
        }while(len_recv>0);

        cout << "Connection Ternimated" << endl;

        close(fd_peer);

    }

    close(fd_self);


    



}

int ProcessCMD(const char* input,char* reply){
    bzero(reply,max_buf);
    stringstream ss;
    ss << string(input);
    string cmd;
    ss >> cmd;
    cout << "CMD:" << cmd << endl;
    if(cmd=="EXIT"){
        return 0;
    }
    else{
        string op_str;
        ss >> op_str;
        int opnd = atoi(op_str.c_str());
        cout << "Opnd:" << opnd << endl;
        if(cmd=="SET"){
            accu = opnd;           
        }
        else if(cmd=="ADD"){
            accu += opnd;
        }
        else if(cmd=="SUB"){
            accu -= opnd;
        }
        sprintf(reply,"%d\r\n",accu);
        return 1;
    }
}


