
#include "iostream"
#include "unistd.h"
#include "arpa/inet.h"
#include "cstdio"
#include "cstring"
#include "cstdlib"
#include "signal.h"
#include "errno.h"
#include "writeline.h"

using namespace std;


int main(int argc,char** argv){
	if(argc < 3){
		cout << "usage : client [server ip] [server port]" << endl;
		exit(1);
	}

	// create socket
	int fd = socket(AF_INET,SOCK_STREAM,0);
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET,argv[1],&addr.sin_addr);
	// connect
	if(connect(fd,(sockaddr*)&addr,addr_len)){
		cout << "connect error" << endl;
		exit(1);
	}
	char c;
	char buf[MAX_BUF_SIZE];
	while((c = getc(stdin))!=EOF){
		if(c == '@'){
			c = '\n';
		}
		else if(c == '\n'){
			c = '@';
		}
		writeline(fd,c);
		if(c=='\n'){
			bzero(buf,MAX_BUF_SIZE);
			read(fd,buf,MAX_BUF_SIZE);
			cout << "Reply from server : " << buf;
		}
	}
	close(fd);
	
}


