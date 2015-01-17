#include "cstdio"
#include "pthread.h"
#include "cstdlib"
using namespace std;

#define MAX_BUF_SIZE 2048

struct Wline{
	char buf[MAX_BUF_SIZE];
	char* ptr;
	int init;
};

static pthread_once_t w1_once;
static pthread_key_t w1_key;

void writeline_dtor(void* ptr){
	free(ptr);
}
void writeline_once(){
	pthread_key_create(&w1_key,writeline_dtor);
}
void writeline_r(int sockfd,char c){
	Wline* pWl;
	// once
	pthread_once(&w1_once,writeline_once);
	// create struct if not exist
	if((pWl = (Wline*)pthread_getspecific(w1_key))==NULL){
		pWl = (Wline*)malloc(sizeof(Wline));
		pWl->init = 1;
		pWl->ptr = pWl->buf;
		pthread_setspecific(w1_key,pWl);
	}
	Wline& wl = *pWl;
	// check initialized
	if(!wl.init){
		wl.init = 1;
		wl.ptr = wl.buf;
	}
	// append to back
	*wl.ptr++ = c;
	// see if word is new line
	if(c=='\n'){
		// append \0
		*wl.ptr = '\0';
		cout << "Really use write to send : " << wl.buf;
		write(sockfd,wl.buf,wl.ptr - wl.buf);
		wl.ptr = wl.buf;
	}
}
