#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>
#include "pthread.h"

using namespace std;

#define MAX_BUF_SIZE 2048

struct Rline{
    char* ptr;
    char buf[MAX_BUF_SIZE];
    int count;
};
static Rline rl;

void serveClient(int fd);
int readline(int fd,char* buf,size_t max_size);
int main(int argc,char** argv){

    if(argc < 2){
        cerr << "usage : server [port]" << endl;
        exit(1);
    }
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
    addr_self.sin_port = htons(atoi(argv[1]));


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cout << "bind error" << endl;
        return 1;
    }

    if(listen(fd_self,10)){
       cout << "listen error" << endl; 
        return 1;
    }
    

    while(true){

        fd_peer = accept(fd_self,(sockaddr*)&addr_peer,&len_cli);
        if(fd_peer < 0){
            cout << "accept error" << endl;
        }
        else{
            serveClient(fd_peer);
        }

    }
    close(fd_self);
}

void serveClient(int fd){
    cout << "Accept new connection" << endl;
    
    // init global rl
    rl.count = 0;
    rl.ptr = NULL;
    
    char buf[MAX_BUF_SIZE];
    int len_recv;
    do{
        bzero(buf,MAX_BUF_SIZE);
        len_recv = readline(fd,buf,MAX_BUF_SIZE-1);
        //cout << "Recv length:" << len_recv << endl;
        /*
        for(int i=0;i<len_recv;i++){
            if(buf[i]=='\n'){
                buf[i] = '@';
            }
        }
        */
        cout << "Readed:" << buf << endl;
        buf[MAX_BUF_SIZE-1] = '\0';

    }while(len_recv>0);

    cout << "Client Ternimated" << endl;

    close(fd);

}


int my_read(int fd,char* buf){
    if(rl.count <= 0){
        // not enough
        rl.count = read(fd,rl.buf,MAX_BUF_SIZE);
        if(rl.count <= 0){ // error state
            return rl.count;
        }
        // success
        rl.ptr = rl.buf;
    }
    *buf = *rl.ptr++; 
    --rl.count;
    return 1;  
}

// read a line from socket fd, but not exceeds max_size's characters
int readline(int fd,char* buf,size_t max_size){
    int total_n = 0,read_n;
    char* ptr = buf;
    while(max_size > 0){
        read_n = my_read(fd,ptr);
        if(read_n <= 0){ // an error occur, return that code back
            return read_n;
        }
        else{
            max_size--;
            total_n++;
            if(*ptr++ == '\n'){
                break;
            }
        }
    }
    return total_n;
}








