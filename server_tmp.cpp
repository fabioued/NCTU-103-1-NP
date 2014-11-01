#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>

using namespace std;


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
    addr_self.sin_port = htons(50000);


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cout << "bind error" << endl;
        return 1;
    }

    if(listen(fd_self,10)){
       cout << "listen error" << endl; 
        return 1;
    }
    
    const int max_buf = 2048;
    char buf[max_buf];

    while(true){

        fd_peer = accept(fd_self,(sockaddr*)&addr_peer,&len_cli);
        if(fd_peer==-1){
            cout << "accept error" << endl;
            return 1;
        }
        cout << "Accept new connection" << endl;
        bzero(buf,max_buf);
        
        int len_recv;
        do{
            len_recv = read(fd_peer,buf,max_buf-1);
            cout << "Recv length:" << len_recv << endl;
            cout << "Readed:" << buf << endl;
            buf[max_buf-1] = '\0';

        }while(len_recv>0);

        cout << "Client Ternimated" << endl;

        close(fd_peer);

    }

    close(fd_self);


    



}
