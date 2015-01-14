#include <string>
#include "errno.h"
#include <iostream>
#include "netdb.h"
#include <sstream>

#include "countTCP.h"

using namespace std;

int socketInit(const char* hostname,unsigned short port);
void mainLoop(int fd);

int main(int argc,char** argv){
#ifndef VERBOSE
    freopen("Log.client","a",stderr);
#endif
    if(argc < 3){
        cout << "usage : ./client.out [server_address] [server_port]" << endl;
        exit(1);
    }   
    int fd = socketInit(argv[1],atoi(argv[2]));
    mainLoop(fd);
    close(fd);
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

void mainLoop(int fd){
    
    fd_set rset,allset;
    FD_ZERO(&allset);
    FD_SET(fd,&allset);
    FD_SET(FD_STDIN,&allset);
    int maxfdp1 = max(fd,FD_STDIN) + 1;
    int nready;
    bool stdinClose = false;
    Entry ent;
    while(true){
        rset = allset;
        nready = select(maxfdp1,&rset,NULL,NULL,NULL);
        if(nready > 0){
            // stdin
            if(FD_ISSET(FD_STDIN,&rset)){
                int number;
                cin >> number;
                if(!cin){
                    // eof reached, close stdin
                    stdinClose = true;
                    cout << "EOF reached" << endl;
                    break;
                }
                else{
                    ent.type = T_NUM;
                    ent.num = number;
                    writeObject(fd,ent);
                }
            }
            // socket
            if(FD_ISSET(fd,&rset)){
                // read it !
                switch(readObject(fd,ent)){
                case E_CLOSED: case E_PREMATURE:
                    cerr << "[WARNING] Server closed" << endl;
                    return;
                    break;
                case E_ERROR:
                    cerr << "[SEVERE] Unknown error" << endl;
                    return;
                    break;
                case E_SUCCESS:
                    if(ent.type == T_WARN){
                        // warn message
                        ent.type = T_NUM;
                        // re-send
                        writeObject(fd,ent);
                    }
                    else if(ent.type == T_SUM){
                        // sum
                        cout << "Sum " << ent.num << endl;
                    }
                    else{
                        cerr << "[SEVERE] Unknown command from server" << endl;
                    }
                    break;

                    
                }


            }
        }
        else{
            cout << "Select with Error : " << nready << endl;
        }
    }
}
