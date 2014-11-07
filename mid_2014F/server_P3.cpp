#include "sys/socket.h"
#include "arpa/inet.h"
#include "netinet/in.h"
#include "unistd.h"
#include "signal.h"

#include "cstdlib"
#include <cstring>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>


#define MAX_SZ 2048
using namespace std;


int ProcessCMD(int fd_peer,char* v_buf){
	vector<int> opnds;
	string cmd;
	string input(v_buf);
	stringstream ss;
	ss << input;
	ss >> cmd;
	if(cmd=="EXIT"){
		return 1;		
	}
	int times;
	ss >> times;
	string token;
	for(int i=0;i<times;i++){
		ss>>token;			
		int opnd = atoi(token.c_str());
		opnds.push_back(opnd);			
	}
	int result = -9999;
	if(cmd=="ADD"){
		result = 0;
		for(int x:opnds){
			cout << "opnd:" << x << endl;
			result += x;			
		}			
	}
	else if(cmd=="MUL"){
		result = 1;				
		for(int x:opnds){
			cout << "opnd:" << x << endl;
			result *= x;			
		}			
	}
	char buf[MAX_SZ];
	bzero(buf,MAX_SZ);
	snprintf(buf,MAX_SZ,"%d\r\n",result);	
	write(fd_peer,buf,MAX_SZ);
	return 0;
}



void ProcessClient(int fd_peer){
		char buf[MAX_SZ];
		int recv;
		do{	
				bzero(buf,MAX_SZ);
				recv = read(fd_peer,buf,MAX_SZ);
				cout << "Read:" << buf << endl;
				int stat = ProcessCMD(fd_peer,buf);
				if(stat > 0){
						cout << "EXIT SINGAL!" << endl;
						break;			
				}
		}while(recv>0);
}


struct FD_T{
	int fd;
	FD_T(){
		fd = -1;			
	}		
};

int main(){
	
	
	sockaddr_in addr_self,addr_peer;
	int fd_self,fd_peer;
	
	socklen_t addr_sz = sizeof(addr_peer);
	fd_self = socket(AF_INET,SOCK_STREAM,0);
	
	if(fd_self < 0){
		cout << "socket error" << endl;			
	}			

	addr_self.sin_family = AF_INET;
	addr_self.sin_addr.s_addr = INADDR_ANY;
	addr_self.sin_port = htons(5000);

	if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
			cout << "bind error" << endl;
			}
	if(listen(fd_self,10)){
			cout << "listen error" << endl;
			}
				
	char buf[MAX_SZ];
	
	FD_T fd_array[MAX_SZ];
	int next_index = 0;

	fd_set rset,allset;
	FD_ZERO(&allset);
	FD_SET(fd_self,&allset);
	int max_fd = fd_self;
	while(true){
			rset = allset;
			int nready = select(max_fd+1,&rset,NULL,NULL,NULL);
			cout << "Ready:" << nready << endl;
			// check new 
			
			if(FD_ISSET(fd_self,&rset)){
				fd_peer = accept(fd_self,(sockaddr*)&addr_peer,&addr_sz);
				int index = -1;
				for(int i=0;i<next_index;i++){
					if(fd_array[i].fd<0){
						index = i;
						break;	
					}		
				}
				if(index < 0){
					if(next_index < MAX_SZ){
						index = next_index;
						next_index++;		
					}			
				}
				fd_array[index].fd = fd_peer;
				max_fd = max(max_fd,fd_peer);
				FD_SET(fd_peer,&allset);			
			}
			
			// accept
			char buf[MAX_SZ];
			for(int i=0;i<next_index;i++){
				int fd = fd_array[i].fd;
				if(fd<0 || !FD_ISSET(fd,&rset)) continue;
				
				bzero(buf,MAX_SZ);
				int recv = read(fd,buf,MAX_SZ);
				cout << "Read:" << buf << endl;
				int stat = ProcessCMD(fd,buf);
				if(stat > 0){
						cout << "EXIT SINGAL!" << endl;
						fd_array[i].fd = -1;
						close(fd);
						FD_CLR(fd,&allset);
				}
						
			}	
			
	}
	close(fd_self);
}

