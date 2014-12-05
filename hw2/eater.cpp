#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>

using namespace std;


const int MAX_BUF_SIZE = 2048;
char buf[MAX_BUF_SIZE];
char ack_buf[MAX_BUF_SIZE];
int main(int argc,char** argv){
    if(argc < 2){
      cout << "Usage : receiver.out [bind port]" << endl;
      exit(1);
    }

    sockaddr_in addr_self,addr_peer;
    socklen_t len_cli;

    int fd_self,fd_peer;

    fd_self = socket(AF_INET,SOCK_DGRAM,0);

    if(fd_self==0){
        cout << "Socket error." << endl;
        return 1;
    }


    cout << "Prepare to bind UDP port : " << argv[1] << endl;
    addr_self.sin_family = AF_INET;
    addr_self.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_self.sin_port = htons(atoi(argv[1]));


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cout << "bind error" << endl;
        exit(1);
    }
    int recv_n;
    int count = 0;
    cout << "Wait for data..." << endl;
    while(true){
        len_cli = sizeof(addr_peer);
        bzero(buf,MAX_BUF_SIZE);
        recv_n = recvfrom(fd_self,buf,MAX_BUF_SIZE-1,0,(sockaddr*)&addr_peer,&len_cli);
        cout << "Received:" << buf << endl;        
        count++;
        cout << "Data count : " << count << endl;
        snprintf(ack_buf,MAX_BUF_SIZE,"ACK for : %s",buf);
        sendto(fd_self,ack_buf,strlen(ack_buf),0,(sockaddr*)&addr_peer,len_cli);
    }

    close(fd_self);


    



}
