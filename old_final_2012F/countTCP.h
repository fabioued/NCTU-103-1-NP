#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include "fcntl.h"
#include "errno.h"

#define MAX_BUF_SIZE 1024
#define MAX_CONCURRENT_SESSION 2048
#define VERBOSE

#define FD_STDIN 0

#define T_NUM 1
#define T_WARN 2
#define T_SUM 3

#define E_SUCCESS 0
#define E_ERROR 1
#define E_EMPTY 2
#define E_PREMATURE 3
#define E_CLOSED 4

struct Entry{
    int type; // T_NUM , T_WARN, T_SUM
    int num;
};

void setNonBlock(int fd){
    // socket option : non-blocking
    int flag=fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}

template<typename T>
void writeObject(int fd,T& obj){
    write(fd,&obj,sizeof(obj));
}
template<typename T>
int readObject(int fd,T& rObj){
    char* obj = (char*)&rObj;
    size_t size =  sizeof(rObj); 
    char buf[MAX_BUF_SIZE];
    int read_n;
    size_t offset = 0;
    while(size > 0){
        read_n = read(fd,buf,size > MAX_BUF_SIZE ? MAX_BUF_SIZE : size );
        if(read_n < 0){
            if(errno == EWOULDBLOCK){
                if(offset > 0){
                    // Data is comming but not so fast
                    // keep waiting
                    continue;
                }
                else{
                    // no data so far, return as empty data
                    return E_EMPTY;
                }
            }  
            else{
                // other error
                return E_ERROR;
            }
        }
        else if(read_n == 0){
            if(offset > 0 && size > 0){ // prematured close
                return E_PREMATURE;
            }
            else{
                return E_CLOSED;   
            }
        }
        size -= read_n;
        memcpy(obj+offset,buf,read_n);
        offset += read_n;
    }
    return E_SUCCESS;
}

timeval tv_sub(const timeval& lhs,const timeval& rhs){
    timeval ret = lhs;
    if((ret.tv_usec -= rhs.tv_usec) < 0){
        --ret.tv_sec;
        ret.tv_usec += 1000000;
    }
    ret.tv_sec -= rhs.tv_sec;
    return ret;
}




