#include "iostream"
#include "unistd.h"
#include "arpa/inet.h"
#include "cstdio"
#include "cstring"
#include "cstdlib"
#include "signal.h"
#include "errno.h"
#include "pthread.h"
#include "writeline_r.h"

#define MAX_BUF_SIZE 2048

using namespace std;



void* serveClient(void*);
int main(int argc,char** argv){
	if(argc < 2){
		cout << "Usage : server <port>" << endl;
		exit(1);	
	}
	sockaddr_in addr_remote,addr_self;
	socklen_t addr_len;
	int fd;
	pthread_t tid;

	fd = socket(AF_INET,SOCK_STREAM,0);
	addr_self.sin_family = AF_INET;
	addr_self.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_self.sin_port = htons(atoi(argv[1]));
	
	//bind port
	bind(fd,(sockaddr*)&addr_self,sizeof(addr_self));
	
	// listen
	if(listen(fd,10)){
		cout << "listen error" << endl;
	}

	while(true){
		addr_len = sizeof(addr_remote);
		int new_fd = accept(fd,(sockaddr*)&addr_remote,&addr_len);
		if(new_fd > 0){
			cout << "New client arrive with fd : " << new_fd << endl;
			//serveClient(new int(new_fd));
			pthread_create(&tid,NULL,&serveClient,new int(new_fd)); // create new to serve it
		}
	}

	close(fd);
}

void* serveClient(void* ptr){
	pthread_detach(pthread_self());
	int fd = *(int*)ptr;
	delete (int*)ptr;
	char c;	
	// start receive
	while(true){	
		int read_n = read(fd,&c,1);
		if(read_n <= 0){
			cout << "Client close : " << fd << endl;
			break;
		}
		cout << "readed : " << c << endl;
		writeline_r(fd,c);
	}

	close(fd);
	return NULL;
}
