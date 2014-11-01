#include <iostream>
#include "sys/socket.h"
#include "netinet/in.h"
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


const int max_buf = 2048;

int main(){

    sockaddr_in addr_self,addr_peer;
    socklen_t len_peer;

    int fd_peer;

    fd_peer = socket(AF_INET,SOCK_STREAM,0);

    if(fd_peer==0){
        cout << "Socket error." << endl;
        return 1;
    }


    addr_peer.sin_family = AF_INET;
    addr_peer.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",&addr_peer.sin_addr);


    char buf[max_buf];

    while(true){
        
        connect(fd_peer,(sockaddr*)&addr_peer,sizeof(addr_peer));
        if(fd_peer==-1){
            cout << "connect error" << endl;
            return 1;
        }
        cout << "Connected to Server." << endl;

        string input;
        bool inputEnd = false;
        while(true) 
        {
            getline(cin,input);
            if(input.size()<=3){
                continue;
            }

            input.append("\r\n");
            write(fd_peer,input.c_str(),input.size()+1);       
            bzero(buf,max_buf);
            int len_recv = read(fd_peer,buf,max_buf-1);
            //cout << "Recv Length:" << len_recv << endl;
            if(len_recv <= 0){
                break;
            }
            else{
                cout <<buf ;                
            }
            //cout << "---------" << endl;

        }
        inputEnd = true;


        cout << "The server has closed the connection." << endl;

        close(fd_peer);
        break;
    }

    close(fd_peer);


    



}
