#include <iostream>
#include "signal.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_icmp.h"
#include "arpa/inet.h"
#include "unistd.h"
#include "sys/time.h"
#include "cstring"

#include "in_cksum.c"
#define MAX_BUF_SIZE 1500
#define TIME_ALARM 1
#define ICMP_DATA_SIZE 56 
#define ICMP_HEADER_SIZE 8
//#define VERBOSE

typedef void (*Sigfunc)(int);

using namespace std;


int fd;
short icmpID;
int nsent;
char sendbuf[MAX_BUF_SIZE];
char recvbuf[MAX_BUF_SIZE];
char buf[MAX_BUF_SIZE];
sockaddr_in addr,addrRecv;
socklen_t addr_len,addr_lenRecv;

void mainLoop();
void receiveICMP();
void tv_sub(timeval& lhs,const timeval& rhs);

void sendICMP(int signo){
    // set alarm again
    alarm(TIME_ALARM);
    // send icmp
    int len = ICMP_DATA_SIZE + ICMP_HEADER_SIZE;
    // fill data
    icmp* icmpData = (icmp*)sendbuf;
    icmpData->icmp_type = ICMP_ECHO;
    icmpData->icmp_code = 0;
    icmpData->icmp_id = icmpID;
    icmpData->icmp_seq = nsent++;
    memset (icmpData->icmp_data,0xa5,ICMP_DATA_SIZE);
    gettimeofday((timeval*)icmpData->icmp_data,NULL);
    icmpData->icmp_cksum = 0;
    icmpData->icmp_cksum = in_cksum((unsigned short*)icmpData,len);
    // send it
    sendto(fd,sendbuf,len,0,(sockaddr*)&addr,addr_len);
}

int main(int argc,char** argv){
    if(argc < 2){
        cerr << "Usage : ping [ip]" << endl;
        exit(1);
    }
    // Set Alarm
    Sigfunc old_handle = signal(SIGALRM,sendICMP);
    siginterrupt(SIGALRM,1);
    alarm(TIME_ALARM);
    // create socket
    fd = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    setuid(getuid());
    if(fd < 0){
        cerr << "Cannot create socket" << endl;
        exit(1);    
    }
    // icmp id
    icmpID = getpid() & 0xffff;
    // seq #
    nsent = 0;
    // fill target address
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr.sin_addr);
    addr_len = sizeof(addr);

    mainLoop();

    
    close(fd);
    signal(SIGALRM,old_handle);
}




void mainLoop(){
    while(true){
       receiveICMP();

    }
}


void receiveICMP(){
    int read_n;
    ip* ipData;
    icmp* icmpData;
    size_t ipHeaderLen;
    size_t icmpLen;
    timeval tvRecv;
    double rtt;
again:
    addr_lenRecv = sizeof(addrRecv);
    read_n = recvfrom(fd,recvbuf,MAX_BUF_SIZE,0,(sockaddr*)&addrRecv,&addr_lenRecv);
    if(read_n <= 0){
        if(errno == EINTR){
            goto again;
        }
        else{
            cerr << "Unknown recvform error" << endl;
            return;
        }
    }
    ipData = (ip*)recvbuf;
    if(ipData->ip_p != IPPROTO_ICMP){
        cerr << "Protocal is not ICMP" << endl;
        return;
    }
    ipHeaderLen = (ipData->ip_hl) << 2;
    if((icmpLen = read_n - ipHeaderLen) < ICMP_HEADER_SIZE){
        cerr << "ICMP packet header is error" << endl;
        return;
    }
    icmpData = (icmp*)(recvbuf + ipHeaderLen);
    if(icmpData->icmp_type != ICMP_ECHOREPLY){
#ifdef VERBOSE
        cerr << "ICMP type is not ECHOREPLY" << endl;
#endif
        return;
    }
    if(icmpData->icmp_id != icmpID){
        cerr << "ICMP ID mismatch" << endl;
        return;
    }
    if(icmpLen < ICMP_HEADER_SIZE + sizeof(timeval)){
        cerr << "ICMP packet has not enough data to parse" << endl;
        return;
    }
    // extract time   
    timeval& tvSend = *(timeval*)icmpData->icmp_data;
    // compute time diff
    gettimeofday(&tvRecv,NULL);
    tv_sub(tvRecv,tvSend);
    rtt = tvRecv.tv_sec * 1000.0 + tvRecv.tv_usec / 1000.0;
    // extract ip
    inet_ntop(AF_INET,&addrRecv.sin_addr,buf,MAX_BUF_SIZE);
    // show info
    printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n",icmpLen,buf,icmpData->icmp_seq,ipData->ip_ttl,rtt);
    

}

void tv_sub(timeval& lhs,const timeval& rhs){
    if((lhs.tv_usec -= rhs.tv_usec) < 0){
        lhs.tv_usec += 1000000;
        --lhs.tv_sec;
    }
    lhs.tv_sec -= rhs.tv_sec;
}
