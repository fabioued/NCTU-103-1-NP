#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>
#include "netdb.h"
#include "signal.h"
#include "errno.h"

using namespace std;

typedef void (*Sigfunc)(int);
const int MAX_BUF_SIZE = 2048;
char buf[MAX_BUF_SIZE];
void handle_sig_alrm(int signo){
    alarm(1);
    return ;
}


int main(int argc,char** argv){
    // Usage
    if(argc < 3){
      cout << "Usage : sender.out [target address] [connect port]" << endl;
      exit(1);
    }
    
    // Set Alarm
    Sigfunc old_handle = signal(SIGALRM,handle_sig_alrm);
    siginterrupt(SIGALRM,1);
    alarm(1);


    // Get Host
    hostent* host = gethostbyname(argv[1]);
    if(!host){
      cout << "Cannot resolve host." << endl;
      exit(1);
    }


    // Socket Variables
    sockaddr_in addr_self,addr_peer;
    socklen_t len_cli;
    int fd_self,fd_peer;

    // Create Socket
    fd_self = socket(AF_INET,SOCK_DGRAM,0);
    if(fd_self==0){
        cout << "Socket error." << endl;
        return 1;
    }


    // Fill Socket Info
    addr_peer.sin_family = AF_INET;
    addr_peer.sin_port = htons(atoi(argv[2]));
    //memcpy(&addr_peer.sin_addr,host->h_addr,host->h_length);
    memcpy(&addr_peer.sin_addr,*host->h_addr_list,sizeof(in_addr));

    inet_ntop(host->h_addrtype,*host->h_addr_list,buf,MAX_BUF_SIZE);
    cout << "Send to IP:" << buf << " Port : "<< argv[2] << endl;

    
    
    // Send Info Init
    int recv_n;
    
    int finished = 0;
    int i = 0;
    while(!finished){
      bzero(buf,MAX_BUF_SIZE);
      len_cli = sizeof(addr_peer);
      snprintf(buf,MAX_BUF_SIZE,"Data ID %d\r\n",i);
      cout << "Send : " << i << endl;
      sendto(fd_self,buf,strlen(buf),0,(sockaddr*)&addr_peer,len_cli);
      recv_n = recvfrom(fd_self,buf,MAX_BUF_SIZE-1,0,(sockaddr*)&addr_peer,&len_cli);
      if(recv_n < 0){
          if(errno== EINTR){
              cout << "Re-send..." << endl;   
              continue;
          }
          else{
              cout << "Unknown System Error." << endl;
              exit(1);
          }
      }
      i++;
      cout << buf << endl;
      if(i>=10){
          break;
      }
    }
    signal(SIGALRM,old_handle);
    close(fd_self);


    



}
