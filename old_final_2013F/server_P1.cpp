#include "countUDP.h"

using namespace std;

void mainLoop(int fd);

int main(int argc,char** argv){
    if(argc < 2){
      cout << "Usage : server.out [bind port]" << endl;
      exit(1);
    }
    char* v_port = argv[1];

    sockaddr_in addr_self;

    int fd_self;

    fd_self = socket(AF_INET,SOCK_DGRAM,0);

    if(fd_self==0){
        cerr << "Socket error." << endl;
        return 1;
    }


    cerr << "[STATE] Prepare to bind UDP port : " << v_port << endl;
    addr_self.sin_family = AF_INET;
    addr_self.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_self.sin_port = htons(atoi(v_port));


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cerr << "bind error" << endl;
        exit(1);
    }
    mainLoop(fd_self); 

    close(fd_self);

    return 0;
}

void mainLoop(int fd){
    sockaddr_in addr_peer;
    socklen_t len_cli;
    int recv_n;
    bool initialized = false;
    int count = 0;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    char buf[MAX_BUF_SIZE];
    int warnNum;
    while(true){
        len_cli = sizeof(addr_peer);
        bzero(buf,MAX_BUF_SIZE);
        recv_n = recvfrom(fd,buf,MAX_BUF_SIZE-1,0,(sockaddr*)&addr_peer,&len_cli);
        if(recv_n <= 0){
            if(errno == EWOULDBLOCK){
                // timed out
                warnNum = 100 - count;
                bzero(buf,MAX_BUF_SIZE);
                snprintf(buf,MAX_BUF_SIZE,"WARN %d\n",warnNum);
                sendUDP(fd,buf,strlen(buf),&addr_peer);
            }
            else{
                cout << "Unexpected Error." << endl;
                break;
            }
        }
        else{
            cerr << "[READ] " <<buf << endl;
            if(!initialized){
                cerr << "[STATE] Initialized , enable timeout option" << endl;
                // first time , set timeout
                initialized = true;
                tv.tv_sec = 5;
                setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
            }
            int num = atoi(buf);
            count += num;
            cerr << "[STATE] current count : " << count << endl;
            if(count > 99){
                bzero(buf,MAX_BUF_SIZE);
                snprintf(buf,MAX_BUF_SIZE,"Sum %d\n",count);
                count = 0;
                cerr << "[STATE] count reset" << endl;
                sendUDP(fd,buf,strlen(buf),&addr_peer);
                // reset all
                initialized = false;
                tv.tv_sec = 0;
                setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));


            }
        }
    }

    
}
