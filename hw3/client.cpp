#include "non-block-file.h"

#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>


using namespace std;

int socketInit(const char* ip,unsigned short port);
void run_client();
void clientLogin();

int fd_cmd;
const char* name;

int main(int argc,char** argv){
    
    if(argc<4){
        cerr << "Usage : ./client <ip> <port> <username>" << endl;
        exit(1);
    }
    
    fd_cmd = socketInit(argv[1],atoi(argv[2]));   
    name = argv[3];
    run_client();

    close(fd_cmd);

}
int socketInit(const char* hostname,unsigned short port){
    struct hostent* host;

    host = gethostbyname(hostname);

    if(!host){
        cout << "Host name error" << endl;
        exit(1);
    }
    sockaddr_in addr_self;
    int fd_self;

    addr_self.sin_family = PF_INET;
    addr_self.sin_port = htons(port);
    memcpy(&addr_self.sin_addr,host->h_addr,host->h_length);

    fd_self = socket(AF_INET,SOCK_STREAM,0);

    if(fd_self < 0){
        cout << "create socket failed." << endl;
        exit(1);
    }

    if(connect(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))<0){
        cout << "connect failed." << endl;
        exit(1);
    }


    return fd_self;


}


void run_client(){
    clientLogin();

    // use select to read from stdin and socket
    fd_set allset,rset;

}

// send login message to server
void clientLogin(){
    // send login header
    cmdHeader header;
    header.cmdType = CMD_LOGIN;
    header.setName(name);
    sendObject(fd_cmd,&header,sizeof(header));
    // retrive response
    readObject(fd_cmd,&header,sizeof(header));
    if(header.cmdType == CMD_LIST){
        printf("Welcome to the dropbox-like server! : %s\n",name);
    }
}


