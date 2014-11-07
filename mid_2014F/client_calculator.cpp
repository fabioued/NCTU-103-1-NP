#include "sys/socket.h"
#include "arpa/inet.h"
#include "netinet/in.h"
#include "unistd.h"
#include "netdb.h"

#include "cstdlib"
#include <cstring>
#include <iostream>
#include <cstdio>


#define MAX_SZ 2048
using namespace std;


int main(){
		sockaddr_in addr_self,addr_peer;
		socklen_t addr_sz = sizeof(addr_peer);
		int fd_self,fd_peer;

		fd_self = socket(AF_INET,SOCK_STREAM,0);

		hostent* host;
		char hostname[MAX_SZ] = "127.0.0.1";
		host = gethostbyname(hostname);

		if(!host){
			cout << "host error" << endl;
			return 0;
				}
		addr_peer.sin_family = AF_INET;
		addr_peer.sin_port = htons(5000);
		memcpy(&addr_peer.sin_addr,host->h_addr,host->h_length);


		if(connect(fd_self,(sockaddr*)&addr_peer,addr_sz)){
					cout << "connect error" << endl;
				}
		
		
		string input;
		char buf[MAX_SZ];
		while(getline(cin,input)){
				write(fd_self,input.c_str(),input.size());
				bzero(buf,MAX_SZ);
				int recv;
				recv = read(fd_self,buf,MAX_SZ);
				if(recv > 0){
					cout << buf;	
				}
				else{
					cout << "The server has closed the connection." << endl;
					break;
				}						
		}
		close(fd_self);


			
			
}
