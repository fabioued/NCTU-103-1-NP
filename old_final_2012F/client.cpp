#include "countUDP.h"
#include "netdb.h"
#include <sstream>

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

    fd_self = socket(AF_INET,SOCK_DGRAM,0);

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
    char buf[MAX_BUF_SIZE];
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
                    // send number to server
                    bzero(buf,MAX_BUF_SIZE);
                    snprintf(buf,MAX_BUF_SIZE,"%d",number);
                    write(fd,buf,strlen(buf));  
                }
            }
            // socket
            if(FD_ISSET(fd,&rset)){
                // read it !
                bzero(buf,MAX_BUF_SIZE);
                int read_n = read(fd,buf,MAX_BUF_SIZE);
                if(read_n <= 0){
                    cout << "Server Closed" << endl;
                    break;
                }
                else{
                    cerr << "[READ] " << buf;
                    // parsing
                    stringstream ss;
                    int readNum; 
                    string cmd,numStr;
                    ss << buf;
                    ss >> cmd >> numStr;
                    readNum = stoi(numStr);
                    if(cmd=="Sum"){
                        cout << "Sum is " << readNum << endl;
                    }
                    else if(cmd=="WARN")  
                    {
                        cerr << "[WARN] Reply : " << readNum << endl;
                        // send number to server
                        bzero(buf,MAX_BUF_SIZE);
                        snprintf(buf,MAX_BUF_SIZE,"%d",readNum);
                        write(fd,buf,strlen(buf));  
                    }
                    
                }


            }
        }
        else{
            cout << "Select with Error : " << nready << endl;
        }
    }
}
