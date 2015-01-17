#include "iostream"
#include "unistd.h"
#include "arpa/inet.h"
#include "cstdio"
#include "cstring"
#include "cstdlib"
#include "signal.h"
#include "errno.h"
#include "pthread.h"
#include "time.h"

#define MAX_BUF_SIZE 2048

using namespace std;

sockaddr_in addr_remote;
socklen_t addr_len;
int fd;

void* receive(void*);
void bufclear(char* buf);
void main_loop();
int main(int argc,char** argv){
	if(argc < 3){
		cout << "usage : client <server ip> <server port>" << endl;
		exit(1);
	}
	srand(time(NULL));
	pthread_t tid;
	// create socket
	fd = socket(AF_INET,SOCK_DGRAM,0);
	// filling server info
	addr_remote.sin_family = AF_INET;
	addr_remote.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET,argv[1],&addr_remote.sin_addr);
	addr_len = sizeof(addr_remote);
	// create listening thread
	pthread_create(&tid,NULL,&receive,NULL);
	// goto main loop
	main_loop();
	close(fd);
	return 0;
	
}

void main_loop(){
	char buf[MAX_BUF_SIZE];
	int num;
	int wait;
	// read from stdin
	while(cin >> num){
		bufclear(buf);
		snprintf(buf,MAX_BUF_SIZE,"%d",num);
		sendto(fd,buf,strlen(buf),0,(sockaddr*)&addr_remote,addr_len);
		printf("send %d\n",num);
		wait = rand()%5 + 1;
		printf("wait %ds\n",wait);
		sleep(wait);
	}
}

void* receive(void*){
	pthread_detach(pthread_self());	
	// endless listen!
	char buf[MAX_BUF_SIZE];
	int read_n;
	while(true){
		bufclear(buf);
		addr_len = sizeof(addr_remote);
		read_n = recvfrom(fd,buf,MAX_BUF_SIZE,0,(sockaddr*)&addr_remote,&addr_len);
		if(read_n < 0){
			cout << "unknown error when receive" << endl;
			continue;
		}
		else{
			// show the number
			printf("recv %s\n",buf);	
		}
	}


	return NULL;
}









void bufclear(char* buf){
	bzero(buf,MAX_BUF_SIZE);
}
