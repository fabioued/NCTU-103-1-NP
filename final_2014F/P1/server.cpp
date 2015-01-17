#include "iostream"
#include "unistd.h"
#include "arpa/inet.h"
#include "cstdio"
#include "cstring"
#include "cstdlib"
#include "signal.h"
#include "errno.h"

#define ALARM_TIME 3
#define MAX_BUF_SIZE 2048

using namespace std;

sockaddr_in addr_remote,addr_self;
socklen_t addr_len;
int fd;
// global sum
int sum;
bool init;
int count;

void reset();
void timedout(int signo);
void main_loop();
void bufclear(char* buf);
void processNumber(int n);

int main(int argc,char** argv){
	if(argc < 2){
		cout << "Usage : server <port>" << endl;
		exit(1);	
	}

	// set alarm
	signal(SIGALRM,timedout);
	siginterrupt(SIGALRM,1);
	// create socket
	
	fd = socket(AF_INET,SOCK_DGRAM,0);
	addr_self.sin_family = AF_INET;
	addr_self.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_self.sin_port = htons(atoi(argv[1]));
	
	//bind port
	bind(fd,(sockaddr*)&addr_self,sizeof(addr_self));
	
	main_loop();

	close(fd);
}


void main_loop(){
	reset(); // reset all states	
	char buf[MAX_BUF_SIZE];
	int read_n;
	// start receive
	while(true){
		bufclear(buf);
		addr_len = sizeof(addr_remote);
		read_n = recvfrom(fd,buf,MAX_BUF_SIZE,0,(sockaddr*)&addr_remote,&addr_len);
		if(read_n < 0){
			if(errno == EINTR){
				// interrupted			
			}
			else{
				cout << "Error occur , no = " << errno << endl;	
			}
			continue;
		}
		processNumber(atoi(buf));
		
	}
}

void timedout(int signo){
	cout << "Timed out" << endl;
	reset();
}

void reset(){
	cout << "Reset " << endl;
	sum = 0;
	init = false;	
	int count = 0;
	alarm(0); // close alarm	
}
void bufclear(char* buf){
	bzero(buf,MAX_BUF_SIZE);
}

void processNumber(int n){
	// check init
	if(!init){
		init = true;
		alarm(ALARM_TIME);
		sum = 0;
		count = 0;
	}
	++count;
	sum += n;
	if(count >= 2){
		char buf[MAX_BUF_SIZE];
		bufclear(buf);
		snprintf(buf,MAX_BUF_SIZE,"%d",sum);
		cout << "return sum : " << sum << endl;
		sendto(fd,buf,strlen(buf),0,(sockaddr*)&addr_remote,sizeof(addr_remote));
		reset();
	}
	
}

