#include "countTCP.h"

#include <string>
#include "errno.h"
#include <iostream>
#include "sys/time.h"

#include "vector"
#include "queue"

using namespace std;

void mainLoop(int fd);


struct ClientData{
    int sum;
    int fd;
    bool initialized;
    timeval lastReply;
    // functions
    void clear();
    void readEntry(const Entry& ent);
    void close();
    void updateTime(const timeval& tv);
    ClientData(){ clear(); };
    ~ClientData();
};

ClientData::~ClientData(){
    
}


void ClientData::clear(){
    lastReply.tv_sec = 0;
    lastReply.tv_usec = 0;
    initialized = false;
    fd = -1;
}

void ClientData::readEntry(const Entry& ent){
    if(ent.type != T_NUM){
        cerr << "[SEVERE] Unknown command from client fd : " << fd << endl;
        return;
    }
    if(!initialized){
        initialized = true;
        gettimeofday(&lastReply,NULL);
        sum = 0;
    }
    // update reply time
    gettimeofday(&lastReply,NULL);
    // add sum
    sum += ent.num;
    // check overflow 
    fprintf(stderr,"[INFO] New data entry from client %d, data is %d, current sum is %d \n",fd,ent.num,sum);
    if(sum > 99){
        Entry ret;
        ret.type = T_SUM;
        ret.num = sum;
        sum = 0;
        initialized = false;
        writeObject(fd,ret);
    }
}
void ClientData::close(){
    ::close(fd);
    clear();
}
void ClientData::updateTime(const timeval& tv){
    // check time is expired?
    timeval diff = tv_sub(tv,lastReply);
    if(diff.tv_sec >= 5){
        // timedout!
        cerr << "[TIMEDOUT] Client timedout with fd : " << fd << endl;
        // update lastReply, avoid contiguous warn messsage
        gettimeofday(&lastReply,NULL);
        Entry ent;
        ent.type = T_WARN;
        ent.num = 100 - sum;
        writeObject(fd,ent);
    }
}

struct ClientList{
    void addClient(int fd);
    ClientData data[MAX_CONCURRENT_SESSION];
    int end = 0;
};

void ClientList::addClient(int fd){
    int pos = -1;
    for(int i=0;i<MAX_CONCURRENT_SESSION;i++){
        if(data[i].fd < 0){
            pos = i;
            break;
        }
    }
    if(pos < 0){
        cerr << "[SEVERE] Max concurrent client limit reached" << endl;
        close(fd);
    }
    else{
        end = max(end,pos+1);
        cerr << "[INFO] New client arrive with fd : " << fd << endl;
        data[pos].clear();
        data[pos].fd = fd;
    }
}



ClientList* pClients;


int main(int argc,char** argv){
    if(argc < 2){
      cout << "Usage : server.out [bind port]" << endl;
      exit(1);
    }
    
    char* v_port = argv[1];

    sockaddr_in addr_self;

    int fd_self;

    fd_self = socket(AF_INET,SOCK_STREAM,0);

    if(fd_self==0){
        cerr << "Socket error." << endl;
        return 1;
    }


    cerr << "[STATE] Prepare to bind TCP port : " << v_port << endl;
    addr_self.sin_family = AF_INET;
    addr_self.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_self.sin_port = htons(atoi(v_port));


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cerr << "bind error" << endl;
        exit(1);
    }
    
    if(listen(fd_self,10)<0){
        cout << "listen failed" << endl;
        exit(1);
    }
    
    pClients = new ClientList;
    setNonBlock(fd_self);
    mainLoop(fd_self); 
    delete pClients;

    close(fd_self);

    return 0;
}



void mainLoop(int fd_listen){
    sockaddr_in addr_peer;
    char buf[MAX_BUF_SIZE];
    timeval tv;
    while(true){
        socklen_t len_peer = sizeof(addr_peer);
        // try to accept
        int fd_new = accept(fd_listen,(sockaddr*)&addr_peer,&len_peer);
        if(fd_new < 0){
            // error accept
            if(errno != EWOULDBLOCK){
                cerr << "[SEVERE] Unknown accept error : " << errno << endl;
            }
        }
        else{
            // this accept will not be block, means that a real client is ready
            setNonBlock(fd_new);
            pClients->addClient(fd_new); 
        }
        // scan for each 
        for(int i=0;i<pClients->end;i++){
            auto& client = pClients->data[i];
            if(client.fd >= 0){
                // this is a valid client
                // try to read it!!
                Entry ent;
                switch(readObject(client.fd,ent)){
                case E_ERROR:
                    cerr << "[SEVERE] Read error on fd : " << client.fd << endl;
                    break;
                case E_EMPTY:
                    // client no data
                    // do nothing
                    break;
                case E_PREMATURE:
                    cerr << "[SEVERE] client closed prematurely with fd : " << client.fd << endl;
                    client.close();
                    break;
                case E_CLOSED:
                    client.close();
                    break;
                case E_SUCCESS:
                    client.readEntry(ent);
                    break;
                }
                if(client.initialized){
                    gettimeofday(&tv, NULL);
                    client.updateTime(tv);
                }

            }
        }           
    }
}




