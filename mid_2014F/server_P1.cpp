#include "sys/socket.h"
#include "arpa/inet.h"
#include "netinet/in.h"
#include "unistd.h"

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
	while(true){
			fd_peer = accept(fd_self,(sockaddr*)&addr_peer,&addr_sz);
			if(fd_peer<0){
				cout << "accept error" << endl;
			}
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
			cout << "Client closed" << endl;
			close(fd_peer);
	}
	close(fd_self);
}

