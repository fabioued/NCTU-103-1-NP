#include "cstdio"
using namespace std;

#define MAX_BUF_SIZE 2048

struct Wline{
	char buf[MAX_BUF_SIZE];
	char* ptr;
	int init;
};

static Wline wl;

void writeline(int sockfd,char c){
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
