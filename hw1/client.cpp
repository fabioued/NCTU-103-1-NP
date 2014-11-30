#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>

#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>

#define MAX_BUF_SIZE 2048

using namespace std;

int socketInit(const char* ip,unsigned short port);
int processCMD(int fd_self);
int main(int argc,char** argv){
    
    if(argc<3){
        cout << "Wrong arguments" << endl;
        exit(1);
    }
    
    int fd_self = socketInit(argv[1],atoi(argv[2]));   

    processCMD(fd_self);

    close(fd_self);

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

int processCMD(int fd_self){
    
    FILE* fp = stdin;
    int fd_in = fileno(fp);
    int max_fd = max(fd_self,fd_in);
    bool stdoff = false;
    fd_set rset,allset;
    FD_ZERO(&allset);
    FD_SET(fd_self,&allset);
    FD_SET(fd_in,&allset);

    char buf[MAX_BUF_SIZE];
    while(true){
        //sleep(1);
        rset = allset;
        //cout << "Selecting..." << endl;
        select(max_fd+1,&rset,NULL,NULL,NULL);
        // detect from net
        if(FD_ISSET(fd_self,&rset)){
            //cout << "Data from net" << endl;
            bzero(buf,MAX_BUF_SIZE);
            int read_n = read(fd_self,buf,MAX_BUF_SIZE-1);
            if(read_n <= 0){
                if(!stdoff){
                    cout << "Server close prematurely." << endl;
                    exit(1);
                }    
                else{
                    break;
                }
            }
            cout << buf ;
        }
        // detect from stdin
        if(FD_ISSET(fd_in,&rset)){
            //cout << "Data from stdin" << endl;
            bzero(buf,MAX_BUF_SIZE);
            int read_n = read(fd_in,buf,MAX_BUF_SIZE-1);
            if(read_n <= 0 || strncmp(buf,"exit",4)==0){
                stdoff = true;
                FD_CLR(fd_in,&allset);
                shutdown(fd_self,SHUT_WR);
                //cout << "Client side ended" << endl; 
            }
            else{
                write(fd_self,buf,strlen(buf));
            }
        }
    }


    return 0;
}
