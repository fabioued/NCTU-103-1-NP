#include "iostream"
#include "unistd.h"
#include "arpa/inet.h"
#include "cstdio"
#include "cstring"
#include "cstdlib"
#include "signal.h"
#include "errno.h"
#include "netdb.h"
#include "sys/time.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_icmp.h"


#define ALARM_TIME 1
#define MAX_BUF_SIZE 2048

#define ICMP_DATA_SIZE 56
#define ICMP_HEADER_SIZE 8
#define MAX_PING_IP 254

using namespace std;


struct PingData{
	sockaddr_in addr;
	bool send;
	bool recv;
	timeval sendtime;
	PingData(){
		send = false;
		recv = false;
	}
};

sockaddr_in addr_send,addr_recv;
socklen_t sendaddr_len,recvaddr_len;
int fd;
int nextIP;
PingData* pingData;

char sendbuf[MAX_BUF_SIZE];
char recvbuf[MAX_BUF_SIZE];
char ipBuf[MAX_BUF_SIZE];
const char* subnet;


void tv_sub(timeval& lhs,const timeval& rhs);
void sendICMP(int signo);
void recvICMP();
void main_loop();
void bufclear(char* buf);
void fillHostname(sockaddr_in& addr,char** dest);
bool checkTimedout(); // when no further thing to do, return false : all ip received or timedout
unsigned short in_cksum(unsigned short * addr,int len);

int main(int argc,char** argv){
	if(argc < 2){
		cout << "Usage : pingsubnet <subnet IP>" << endl;
		exit(1);	
	}
	// init data
	subnet = argv[1];
	nextIP = 1;
	pingData = new PingData[MAX_PING_IP + 1];
	// set alarm
	signal(SIGALRM,sendICMP);
	siginterrupt(SIGALRM,1);
	// create socket
	
	fd = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
	// receive timeout is 1 second
	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
	setuid(getuid());
	if(fd < 0){
		cout << "Cannot create socket " << endl;
		exit(1);
	}	
	
	
	alarm(1);
	main_loop();
	delete[] pingData;
	close(fd);
}


void main_loop(){
	while(true){
		recvICMP();
		if(!checkTimedout()){
			break;
		}
	}
}

bool checkTimedout(){
	bool allFinish = true;
	timeval tv;
	char* hostname;
	char ipBuf[MAX_BUF_SIZE];
	// scan all array
	for(int i=1;i<=MAX_PING_IP;i++){
		auto& entry = pingData[i];
		if(entry.send){
			if(!entry.recv){
				allFinish = false;
				// check timeval
				gettimeofday(&tv,NULL);
				tv_sub(tv,entry.sendtime);
				if(tv.tv_sec >= 3){
					//cout << "timedout for " << i << endl;
					// timedout
					entry.recv = true;
					fillHostname(entry.addr,&hostname);
					// ip for show	
					bufclear(ipBuf);
					inet_ntop(AF_INET,&entry.addr.sin_addr,ipBuf,MAX_BUF_SIZE);
					printf("%s",ipBuf);
					if(hostname){
						printf(" (%s)",hostname);	
					}
					printf(" no response\n");
						
				}	
			}
			else{
				//cout << "IP " << i << " Received" << endl;
			}
		}
		else{
			allFinish = false;
		}
	}
	return !allFinish;	
}

void recvICMP(){
	int read_n;
	ip* ipData;
	icmp* icmpData;
	size_t ipHeaderLen;
	size_t icmpLen;
	timeval tvRecv;
	double rtt;
	char* hostname = NULL;
	//
	recvaddr_len = sizeof(addr_recv);
	bufclear(recvbuf);	
	read_n = recvfrom(fd,recvbuf,MAX_BUF_SIZE,0,(sockaddr*)&addr_recv,&recvaddr_len);
	if(read_n < 0){
		if(errno != EWOULDBLOCK && errno != EINTR){
			cout << "Unknown error when receive" << endl;
		}
		return;
	}
	ipData = (ip*)recvbuf;
	if(ipData->ip_p != IPPROTO_ICMP){
		cout << "Protocol is not ICMP" << endl;
		return;
	}
	ipHeaderLen = (ipData->ip_hl) << 2;
	if((icmpLen = read_n - ipHeaderLen) < ICMP_HEADER_SIZE){
		cout << "ICMP packet header size erro " << endl;
		return;
	}
	icmpData = (icmp*)(recvbuf + ipHeaderLen);
	if(icmpData->icmp_type != ICMP_ECHOREPLY){
		return;
	}
	if(icmpLen < ICMP_HEADER_SIZE + sizeof(timeval)){
		cout << "ICMP packet data lost!" << endl;
		return;
	}
	// extract data
	timeval& tvSend = *(timeval*)icmpData->icmp_data;
	// time diff
	gettimeofday(&tvRecv,NULL);
	tv_sub(tvRecv,tvSend);
	rtt = tvRecv.tv_sec * 1000.0 + tvRecv.tv_usec / 1000.0;
	// extract IP for show
	inet_ntop(AF_INET,&addr_recv.sin_addr,ipBuf,MAX_BUF_SIZE);
	// show info
	cout << ipBuf;
	fillHostname(addr_recv,&hostname);
	if(hostname){
		printf(" (%s)",hostname);
	}	
	printf(" RTT=%.3fms\n",rtt);
	// mark received
	// extract last ip
	char* lastNumBuf = ipBuf + strlen(subnet) + 1;
	int lastNum = atoi(lastNumBuf);
	pingData[lastNum].recv = true;
	
}

void sendICMP(int signo){
	// data
	char sendIP[MAX_BUF_SIZE];
	timeval tvSend;
	icmp* icmpData;
	int len = ICMP_HEADER_SIZE + ICMP_DATA_SIZE;
	// check current ID
	if(nextIP >= MAX_PING_IP){
		// no alarm again
		alarm(0);
	}
	else{
		// set alarm again	
		alarm(ALARM_TIME);
	}
	// send icmp
	// generate send IP string
	bufclear(sendIP);
	snprintf(sendIP,MAX_BUF_SIZE,"%s.%d",subnet,nextIP);
	// create send addr
	addr_send.sin_family = AF_INET;
	inet_pton(AF_INET,sendIP,&addr_send.sin_addr);
	sendaddr_len = sizeof(addr_send);
	// record to array	
	gettimeofday(&tvSend,NULL);	
	pingData[nextIP].send = true;
	pingData[nextIP].addr = addr_send;
	pingData[nextIP].sendtime = tvSend;
	// fill icmp data
	icmpData = (icmp*)sendbuf;
	icmpData->icmp_type = ICMP_ECHO;
	icmpData->icmp_code = 0;
	icmpData->icmp_id = 0;
	icmpData->icmp_seq = 0;
	memset(icmpData->icmp_data,0xa5,ICMP_DATA_SIZE);
	memcpy(icmpData->icmp_data,&tvSend,sizeof(tvSend));
	icmpData->icmp_cksum = 0;
	icmpData->icmp_cksum = in_cksum((unsigned short*)icmpData,len);
	// send it
	sendto(fd,sendbuf,len,0,(sockaddr*)&addr_send,sendaddr_len);
	// increase next
	++nextIP;



	

}


void bufclear(char* buf){
	bzero(buf,MAX_BUF_SIZE);
}


unsigned short in_cksum(unsigned short * addr,int len){
	int nleft = len;
	int sum = 0;
	unsigned short* w = addr;
	unsigned short answer = 0;
	
	while(nleft> 1){
		sum += *w++;
		nleft -= 2;
	}
	if(nleft==1){
		*(unsigned char*)(&answer) = *(unsigned char*)w;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
		
}


void tv_sub(timeval& lhs,const timeval& rhs){
	if((lhs.tv_usec -= rhs.tv_usec)<0){
		lhs.tv_usec += 1000000;
		lhs.tv_sec--;
	}	
	lhs.tv_sec -= rhs.tv_sec;
}
void fillHostname(sockaddr_in& addr,char** dest){
	auto pHost = gethostbyaddr(&addr.sin_addr,sizeof(addr.sin_addr),AF_INET);	
	if(pHost){
		*dest = pHost->h_name;
	}
	else{
		*dest = NULL;
	}
		
}
