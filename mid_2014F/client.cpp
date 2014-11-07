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


int main(int argc,char** argv){
		if(argc<4){
			exit(1);		
		}

		sockaddr_in addr_self,addr_peer;
		socklen_t addr_sz = sizeof(addr_peer);
		int fd_self,fd_peer;

		fd_self = socket(AF_INET,SOCK_STREAM,0);

		hostent* host;
		char* hostname = argv[1];
		host = gethostbyname(hostname);

		if(!host){
			cout << "host error" << endl;
			return 0;
				}
		addr_peer.sin_family = AF_INET;
		addr_peer.sin_port = htons(atoi(argv[2]));
		memcpy(&addr_peer.sin_addr,host->h_addr,host->h_length);


		if(connect(fd_self,(sockaddr*)&addr_peer,addr_sz)){
					cout << "connect error" << endl;
				}
		

		
		string input;
		char buf[MAX_SZ];
	
		// send name
		char* name = argv[3];
		int len = strlen(name);
		for(int i=0;i<len;i++){
			if(name[i]=='\n'){
				name[i] = '\0';
				break;		
			}		
		}
		bzero(buf,MAX_SZ);
		snprintf(buf,MAX_SZ,"%s\r\n",name);
		write(fd_self,buf,strlen(buf));

		

		FILE* fp = stdin;
		int fd_in = fileno(fp);

		fd_set rset,allset;
		FD_ZERO(&allset);
		FD_SET(fd_self,&allset);
		FD_SET(fd_in,&allset);
		int max_fdp1 = max(fd_self,fd_in) + 1;
		while(true){
			rset = allset;
			int nready = select(max_fdp1,&rset,NULL,NULL,NULL);
			// from net	
			if(FD_ISSET(fd_self,&rset)){
				int recv;
				bzero(buf,MAX_SZ);
				recv = read(fd_self,buf,MAX_SZ);
				if(recv <= 0){
					cout << "The Server has closed the connection." << endl;
					close(fd_self);
					break;			
				}
				else{
					cout  << buf;		
				}		
			}			
			// from stdin
			if(FD_ISSET(fd_in,&rset))	
			{	
				int recv;
				bzero(buf,MAX_SZ);
				recv = read(fd_in,buf,MAX_SZ);
				if(recv > 0){
					write(fd_self,buf,strlen(buf));	
				}
			}
		}

			
		
		close(fd_self);


			
			
}
