#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>
#include "errno.h"
#include <iostream>
#include "pthread.h"
#include <sys/time.h>

#define FD_STDIN 0
#define MAX_BUF_SIZE 2048
#define MAX_CONCURRENT_SESSION 100

#define VERBOSE


int sendUDP(int fd,const void* buf,size_t sz,sockaddr_in* addr){
    return sendto(fd,buf,sz,0,(sockaddr*)addr,sizeof(*addr));
}

bool compareAddr(sockaddr_in& lhs,sockaddr_in& rhs){
    return lhs.sin_addr.s_addr == rhs.sin_addr.s_addr && lhs.sin_port == rhs.sin_port;
}
