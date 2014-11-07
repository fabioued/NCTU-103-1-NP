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
#include <list>

#ifndef PORT
#define PORT 5000
#endif
#define MAX_SZ 2048
using namespace std;


struct FD_T{
	int fd;
	string name;
	char ip[20];
	unsigned short port;
	FD_T(){
		fd = -1;
		name = "";			
	}		
};

struct OFF_MSG{
	string name,msg,source;
};

list<OFF_MSG> Queue;

FD_T fd_array[MAX_SZ];
int next_index = 0;

void directMSG(FD_T& client,const string& name,const string& msg,bool is_reply = false){
		char buf[MAX_SZ];
		FD_T* target = NULL;
		// search for client
		for(int i=0;i<next_index;i++){
				FD_T& other = fd_array[i];
				cout <<"FD:" << other.fd << " Comapre:" << name << " : " << other.name << endl;
				if(other.fd < 0) continue;
				if(name==other.name){
						target = &other;
						break;		
				}
		}

		if(target){
				// founded
				FD_T& other = *target;
				bzero(buf,MAX_SZ);
				if(!is_reply){
						// to target
						snprintf(buf,MAX_SZ,"%s TOLD you %s\r\n",client.name.c_str(),msg.c_str());
						write(other.fd,buf,strlen(buf));			
						// to self
						bzero(buf,MAX_SZ);
						snprintf(buf,MAX_SZ,"%s has read your message: %s\r\n",other.name.c_str(),msg.c_str());
						write(client.fd,buf,strlen(buf));			
				}
				else{
						// to self
						write(other.fd,msg.c_str(),msg.size());			
						
						}
		}
		else{
				cout << "Offline :" << name << " : " << msg << endl;	
				OFF_MSG obj;
				obj.source = client.name;
				obj.name = name;
				obj.msg = msg;
				Queue.push_back(std::move(obj));
		}

}


int ProcessCMD(FD_T& client,char* v_buf){
	int fd_peer = client.fd;
	string cmd;
	string input(v_buf);
	stringstream ss;
	ss << input;
	ss >> cmd;
	char buf[MAX_SZ];
	if(cmd=="EXIT"){
		return 1;		
	}
	else if(cmd=="WHO"){
		for(int i=0;i<next_index;i++){
			FD_T& client = fd_array[i];
			if(client.fd < 0) continue;		
			bzero(buf,MAX_SZ);
			snprintf(buf,MAX_SZ,"%s %s/%hu\r\n",client.name.c_str(),client.ip,client.port);
			write(fd_peer,buf,strlen(buf));
		}
	}
	else if(cmd=="SAY"){
		string msg;
		char c;
		//ss >> c; // blank
		getline(ss,msg);
		for(int i=0;i<next_index;i++){
			FD_T& other = fd_array[i];
			if(client.fd < 0) continue;		
			bzero(buf,MAX_SZ);
			snprintf(buf,MAX_SZ,"%s SAID %s\r\n",client.name.c_str(),msg.c_str());
			write(other.fd,buf,strlen(buf));
		}
				
	}
	else if(cmd=="TELL"){
		string msg,name;
		ss >> name;
		char c;
		//ss >> c;
		getline(ss,msg);
		directMSG(client,name,msg);		
						
	}
	return 0;
}



void ProcessOffline(FD_T& client){
	auto first = Queue.begin();
	auto last = Queue.end();
	char buf[MAX_SZ];
	while(first!=last){
		auto& obj = *first;
		cout << "Search :" << "From:" << obj.source << " to " << obj.name << "," << obj.msg << endl; 
		if(client.name==obj.name){	
			string& msg = obj.msg;
			bzero(buf,MAX_SZ);
			// to target
			snprintf(buf,MAX_SZ,"%s TOLD you %s\r\n",obj.source.c_str(),msg.c_str());
			write(client.fd,buf,strlen(buf));		
				
			// to self
			bzero(buf,MAX_SZ);
			snprintf(buf,MAX_SZ,"%s has read your message: %s\r\n",client.name.c_str(),msg.c_str());
			directMSG(client,obj.source,string(buf),true);
			
			Queue.erase(first++);
		}
		else{	
			first++;		
		}
	}
			
}



int main(){
	
	
	next_index = 0;
	sockaddr_in addr_self,addr_peer;
	int fd_self,fd_peer;
	
	socklen_t addr_sz = sizeof(addr_peer);
	fd_self = socket(AF_INET,SOCK_STREAM,0);
	
	if(fd_self < 0){
		cout << "socket error" << endl;			
	}			

	addr_self.sin_family = AF_INET;
	addr_self.sin_addr.s_addr = INADDR_ANY;
	addr_self.sin_port = htons(PORT);

	if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
			cout << "bind error" << endl;
			}
	if(listen(fd_self,10)){
			cout << "listen error" << endl;
			}
				
	char buf[MAX_SZ];
	

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
				cout << "New Arrive" << endl;
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
				FD_T& client = fd_array[index];
				client.fd = fd_peer;
				inet_ntop(AF_INET,&addr_peer.sin_addr,client.ip,sizeof(client.ip));
				client.port = ntohs(addr_peer.sin_port);
				cout << "New Client from " << client.ip << ", port:" << client.port << endl;
				printf("port:%hu\n",client.port);
				max_fd = max(max_fd,fd_peer);
				FD_SET(fd_peer,&allset);
				char reply[MAX_SZ];
				bzero(reply,MAX_SZ);
				// get name
				read(fd_peer,buf,MAX_SZ);
				int len = strlen(buf);
				for(int i=0;i<len;i++){
					if(buf[i]=='\r' || buf[i]=='\n'){
						buf[i] = '\0';
						break;		
					}		
				}
				cout << "Client name:" << buf << endl;
				client.name = buf;
				snprintf(reply,MAX_SZ,"***Hi,%s from %s/%hu***\r\n",buf,client.ip,client.port);
				write(fd_peer,reply,strlen(reply));			
				ProcessOffline(client);
			}
			
			// accept
			char buf[MAX_SZ];
			for(int i=0;i<next_index;i++){
				int fd = fd_array[i].fd;
				if(fd<0 || !FD_ISSET(fd,&rset)) continue;
				
				bzero(buf,MAX_SZ);
				int recv = read(fd,buf,MAX_SZ);
				cout << "Read:" << buf << endl;
				int stat = ProcessCMD(fd_array[i],buf);
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

