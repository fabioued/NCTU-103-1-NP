#include "countUDP.h"
#include "vector"
#include "queue"

using namespace std;

void mainLoop();
void* workLoop(void*);

struct DataGram{
    size_t size;
    char buf[MAX_BUF_SIZE];
};

struct ClientData{
    int sum;
    sockaddr_in addr;
    bool initialized;
    queue<DataGram> dataQue;
    pthread_cond_t data_cond;
    pthread_t thread;
    pthread_mutex_t mutex;
    // functions
    void init(sockaddr_in& new_addr);
    void clear();
    ClientData(sockaddr_in& new_addr){ init(new_addr); }
    ClientData(){ clear(); };
    ~ClientData();
};

ClientData::~ClientData(){
    
}

void ClientData::init(sockaddr_in& new_addr){
    clear();
    addr = new_addr;
    sum = 0;
    initialized = true;
}

void ClientData::clear(){
    initialized = false;
    data_cond = PTHREAD_COND_INITIALIZER;
    mutex = PTHREAD_MUTEX_INITIALIZER;
}

struct ClientList{
    int newData(sockaddr_in&,void*,size_t);
    ClientData data[MAX_CONCURRENT_SESSION];
    int data_size = 0;
};

int ClientList::newData(sockaddr_in& addr,void* buf,size_t size){
    // search if this addr is exist
    int id = -1;
    int emptyID = -1;
    bool newOne = true;
    cerr << "[MAIN] New gram arrived " << endl;
    for(int i=0;i<data_size&&newOne;i++){
        auto& entry = data[i];
        pthread_mutex_lock(&entry.mutex);
        if(!entry.initialized){
            emptyID = i;
        }
        if(entry.initialized && compareAddr(entry.addr,addr)){
            // found
            cerr << "[MAIN] This data gram has existed workID : " << i << endl;
            id = i;
            newOne = false;
        }
        pthread_mutex_unlock(&entry.mutex);
    }
    if(newOne){
        // new session, find a place for it
        if(data_size >= MAX_CONCURRENT_SESSION){
            cerr << "[SERVERE] Max concurrent sessions limit reached" << endl;
            return -1;
        }
        if(emptyID < 0){
            // need new space
            emptyID = data_size;
            data_size++;
        }
        auto& entry = data[emptyID];
        entry.init(addr);
        id = emptyID;
        entry.sum = atoi((char*)buf); // initial value
        cerr << "[MAIN] Initial value for id " << emptyID << " is " << entry.sum << endl;
        // new thread created
        pthread_create(&entry.thread,NULL,&workLoop,new int(emptyID));
        return id;  
    }
    else{
        //filling data gram for non-new thread
        auto& entry = data[id];
        DataGram gram;
        gram.size = size;
        bzero(gram.buf,MAX_BUF_SIZE);
        memcpy(gram.buf,buf,size);
        pthread_mutex_lock(&entry.mutex);
        entry.dataQue.push(gram);
        pthread_mutex_unlock(&entry.mutex);
        // we can signal it! so we do it
        pthread_cond_signal(&entry.data_cond);
        return -1;
    }
}

ClientList* pClients;
int fd; // global fd



int main(int argc,char** argv){
    pClients = new ClientList;
    if(argc < 2){
      cout << "Usage : server.out [bind port]" << endl;
      exit(1);
    }
    char* v_port = argv[1];

    sockaddr_in addr_self;

    fd = socket(AF_INET,SOCK_DGRAM,0);

    if(fd==0){
        cerr << "Socket error." << endl;
        return 1;
    }


    cerr << "[STATE] Prepare to bind UDP port : " << v_port << endl;
    addr_self.sin_family = AF_INET;
    addr_self.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_self.sin_port = htons(atoi(v_port));


    if(bind(fd,(sockaddr*)&addr_self,sizeof(addr_self))){
        cerr << "bind error" << endl;
        exit(1);
    }
    mainLoop(); 

    close(fd);
    delete pClients;
    return 0;
}

// main loop : only needs to read 
void mainLoop(){
    sockaddr_in addr_peer;
    socklen_t len_cli;
    char buf[MAX_BUF_SIZE];
    int recv_n;
    while(true){
        len_cli = sizeof(addr_peer);
        bzero(buf,MAX_BUF_SIZE);
        recv_n = recvfrom(fd,buf,MAX_BUF_SIZE-1,0,(sockaddr*)&addr_peer,&len_cli);
        if(recv_n <= 0){
            cout << "Unexpected Error." << endl;
            break;
        }
        else{
            cerr << "[READ] " <<buf << endl;
            pClients->newData(addr_peer,buf,recv_n);
        }
    }

    
}

void* workLoop(void* pData){
    pthread_detach(pthread_self());
    int workID = *(int*)pData;
    delete (int*)pData; // delete data from master
    auto& entry = pClients->data[workID]; 
    timespec ts;
    timeval tv;
    ts.tv_nsec = 0;
    cerr << "[THREAD] New worker thread created with id " << workID << endl; 
    // first
    // deal with data gram
    bool hasData;
    char buf[MAX_BUF_SIZE];
    while(entry.sum <= 99){
        // wait for one gram
        pthread_mutex_lock(&entry.mutex);
        hasData = true;
        cerr << "[THREAD] Wait for new data with id " << workID << endl;
        if(entry.dataQue.empty()){
            cerr << "[THREAD] The queue is empty for id " << workID << endl;
            // no data can extract...
            // wait for new gram
            gettimeofday(&tv, NULL);
            ts.tv_sec = tv.tv_sec + 5;
            int err = pthread_cond_timedwait(&entry.data_cond,&entry.mutex,&ts);
            if(err == ETIMEDOUT){
                cerr << "[THREAD] Timedout for id " << workID << endl;
                // no data from client after 5 seconds
                hasData = false; // do not extract data !
                // send warn to client
                bzero(buf,MAX_BUF_SIZE);
                snprintf(buf,MAX_BUF_SIZE,"WARN %d\n",100 - entry.sum);
                sendUDP(fd,buf,strlen(buf),&entry.addr);
                pthread_mutex_unlock(&entry.mutex);
            }
            else if(err != 0){
                // other error!
                hasData = false;
                cerr << "[SEVERE] Unknown condition error id " << err << " , in work ID : " << workID << endl;
            }
            else{
                // otherwise, main has singal, so that we at least has an data
                cerr << "[THREAD] condition ok for id " << workID << endl;
            }

        }
        else{
            cerr << "[THREAD] The queue has data for id " << workID << endl;
            
        }
        if(hasData){
            // deal one gram
            // pop that gram out
            DataGram gram = entry.dataQue.front();
            entry.dataQue.pop();
            pthread_mutex_unlock(&entry.mutex);
            // deal with gram
            int num = atoi(gram.buf);
            entry.sum += num;
            cerr << "[THREAD] Count with id : " << workID << " is " << entry.sum <<  ", last add is " << num <<  endl;
        }
    }
    // sending final message
    cerr << "[THREAD] Total count exceeds limit with id : " << workID << endl;
    bzero(buf,MAX_BUF_SIZE);
    snprintf(buf,MAX_BUF_SIZE,"Sum %d\n",entry.sum);
    sendUDP(fd,buf,strlen(buf),&entry.addr);
    // cleaning
    pthread_mutex_lock(&entry.mutex);
    cerr << "[THREAD] Working session finished for id " << workID << endl;
    entry.initialized = false ; // mark un-used           
    pthread_mutex_unlock(&entry.mutex);
    return NULL;
}

