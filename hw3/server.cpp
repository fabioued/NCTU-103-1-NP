
#include "iostream"
#include "string"
#include "cstring"
#include "cstdio"
#include "cstdlib"

#include "non-block-file.h"

unsigned short bindPort;

using namespace std;

// function
int socketInit();
void run_server();
// global data
int fd_listen;
FDManager* pFD;
fileManager* pFM;

int main(int argc,char** argv){
    // usage
    if(argc < 2){
        cerr << "Usage : ./server <port>" << endl;
        exit(1);
    }
    // socket init
    bindPort = atoi(argv[1]);   
    fd_listen = socketInit(); 
    // run server
    run_server();
    // ending
    close(fd_listen);
    return 0;
}

void run_server(){
    pFD = new FDManager;
    FDManager& FDm = *pFD;
    FDm.addFD(fd_listen,FD_LISTEN);

    pFM = new fileManager;
    fileManager& fileM = *pFM;

    char buf[MAX_BUF_SIZE];
    fd_set allRset,allWset;
    fd_set rset,wset;
    FD_ZERO(&allRset);
    FD_ZERO(&allWset);
    FD_SET(fd_listen,&allRset);
    int nready;
    sockaddr_in addr_peer;
    socklen_t len_peer;
    while(true){
        rset = allRset;
        wset = allWset;
        nready = select(FDm.maxfdp1,&rset,&wset,NULL,NULL);
        if(nready > 0){
            // foreach FD
            int chk_size = FDm.size;
            for(int i=0;i<chk_size;i++){
                auto& entry = FDm.data[i];
                if(!entry.used){
                    continue;
                }
                if(FD_ISSET(entry.fd,&rset)){
                    // accept socket
                    if(entry.type == FD_LISTEN){
                        len_peer = sizeof(sockaddr_in);
                        int new_fd = accept(fd_listen,(sockaddr*)&addr_peer,&len_peer);
                        FDm.addFD(new_fd,FD_NONE); // new fd added, but no specify type, will read type later
                        FD_SET(new_fd,&allRset);
                        FD_SET(new_fd,&allWset);
                        setNonBlock(new_fd);
                        cerr << "New FD : " << new_fd << " Arrived" << endl;
                    }
                    else{
                    
                        fprintf(stderr,"FD %d is readable!\n",entry.fd);
                        // extract main header
                        int read_n = read(entry.fd,&buf,MAX_BUF_SIZE);
                        fprintf(stderr,"Read size : %d\n",read_n);
                        if(read_n <= 0){
                            cerr << "Connection Closed" << endl;
                            close(entry.fd);
                            FD_CLR(entry.fd,&allRset);
                            FD_CLR(entry.fd,&allWset);
                            entry.remove();
                        }
                        else{
                            if(entry.phase == PHASE_NONE && read_n == sizeof(cmdHeader)){
                                cmdHeader& main_h = *(cmdHeader*)(buf);
                                fprintf(stderr,"PHASE_NONE : Load Header from %s, command code is %d\n",main_h.name,main_h.cmdType);
                                switch(main_h.cmdType){
                                case CMD_LOGIN:
                                    fprintf(stderr,"User '%s' login success\n",main_h.name);
                                    fileM.addUser(main_h.name);
                                    strcpy(entry.name,main_h.name);
                                    entry.responseLogin();
                                    break;
                                }
                                
                            }
                        }

                    }
                }
                // writable?
                if(FD_ISSET(entry.fd,&wset)){ 
                    if(entry.type != FD_LISTEN){
                        entry.flushWrite();
                    }
                }
                
            }

        }
    }
    delete pFM;
    delete pFD;
}






int socketInit(){
    sockaddr_in addr_self;
    int fd_self;

    addr_self.sin_family = PF_INET;
    addr_self.sin_port = htons(bindPort);
    addr_self.sin_addr.s_addr = INADDR_ANY;


    fd_self = socket(AF_INET,SOCK_STREAM,0);

    if(fd_self < 0){
        cout << "create socket failed." << endl;
        exit(1);
    }

    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))<0){
        cout << "bind failed." << endl;
        exit(1);
    }
    if(listen(fd_self,10)<0){
        cout << "listen failed" << endl;
        exit(1);
    }

    return fd_self;
}



/*




FD_ARRAY_T fd_array[MAX_FD_NUM];
int next_index = 0;
int max_fd;
int nready = 0;
fd_set rset,allset;
    
int processWho(FD_ARRAY_T& client);
int processRename(const char* name,FD_ARRAY_T& client);
int processTell(const char* args,FD_ARRAY_T& client);
int processYell(const char* msg,FD_ARRAY_T& client);

int main(int argc,char** argv){
    
    if(argc>=2&&argv[1]){
        bindPort = atoi(argv[1]);    
    }
    else{
        bindPort = DEFAULT_PORT;     
    }

    int fd_self = socketInit(); 
    
    next_index = 0;
    max_fd = fd_self;
    nready = 0;
    FD_ZERO(&allset);
    FD_SET(fd_self,&allset);

    while(true){
        //sleep(1);
        rset = allset;
        nready = select(max_fd+1,&rset,NULL,NULL,NULL);
        // new connection
        if(FD_ISSET(fd_self,&rset)){
            // find position
            int pos = -1;
            for(int i=0;i<next_index;i++){
                if(fd_array[i].fd<0){
                    pos = i;
                    break;
                }
            }
            if(next_index == MAX_FD_NUM){
                cout << "No FDs availalbe." << endl;
            }
            else{
                pos = next_index++;
                sockaddr_in addr_peer;
                socklen_t len_peer = sizeof(addr_peer);
                int new_fd = accept(fd_self,(sockaddr*)&addr_peer,&len_peer);
                FD_ARRAY_T client;
                client.fd = new_fd;
                client.addr = addr_peer;
                bzero(client.ip,MAX_IP_SIZE);
                inet_ntop(AF_INET,&addr_peer.sin_addr,client.ip,sizeof(client.ip));
                client.port = ntohs(addr_peer.sin_port);
                cout << "New Client Arrive:" << client.ip << ", port:" << client.port << " , FD : " << new_fd << endl;
                fd_array[pos] = std::move(client);
                processHello(client);
                max_fd = max(new_fd,max_fd);
                FD_SET(new_fd,&allset);
            }
            
        } // new client if

        for(int i=0;i<next_index;i++){
            FD_ARRAY_T& cli = fd_array[i];
            if(cli.fd < 0){
                continue;
            }
            if(FD_ISSET(cli.fd,&rset)){
                //printf("FD[%d]=%d has data\n",i,cli.fd);
                processClient(fd_self,cli);
            }
        }// for each fd

    } //while
}



int processClient(int fd_self,FD_ARRAY_T& client){
    char buf[MAX_BUF_SIZE];
    bzero(buf,MAX_BUF_SIZE);
    int recv_len = read(client.fd,buf,MAX_BUF_SIZE-1);
    if(recv_len <= 0){
        cout << "Client:["<< client.name << "]Leaves. " << "Connection closed from " << client.ip << ":" << client.port << endl;    
        FD_CLR(client.fd,&allset);
        close(client.fd);
        client.fd = -1;
        processLogout(client);
    }
    else{
        // delete trailing new line
        int len = strlen(buf);
        for(int i=0;i<len;i++){
            if(buf[i]=='\n'){
                buf[i] = '\0';
                break;
            }
        }
        processInput(buf,client);
    }
    return 0;
}
int processHello(FD_ARRAY_T& new_cli){
    char hello[MAX_BUF_SIZE];
    const char* exist = "[Server] Someone is coming!\r\n";    
    sprintf(hello,"[Server] Hello, anonymous! From: %s/%hu\r\n",new_cli.ip,new_cli.port);    
    size_t e_size = strlen(exist);
    size_t h_size = strlen(hello);
    for(int i=0;i<next_index;i++){
        FD_ARRAY_T& client = fd_array[i];
        if(client.fd < 0){
            continue;
        }
        const char* str;
        size_t len;
        if(client.fd==new_cli.fd){
            len = h_size;
            str = hello;
        }
        else{
            len = e_size;
            str = exist;
        }
        //cout << "Write" << str << " To " << client.fd << endl;
        write(client.fd,str,len);
    }
}
int processLogout(FD_ARRAY_T& out_cli){
    char buf[MAX_BUF_SIZE];
    bzero(buf,MAX_BUF_SIZE);
    sprintf(buf,"[Server] %s is offline.\r\n",out_cli.name.c_str());
    for(int i=0;i<next_index;i++){
        FD_ARRAY_T& client = fd_array[i];
        if(client.fd >= 0){
            write(client.fd,buf,strlen(buf));
        }
    }
}
int processInput(char* line,FD_ARRAY_T& client){
    char* cmd = line;
    bool error = false;
    do{
        if(!cmd){
            error = true;
            break;
        }
        else if(strncmp("who",line,3)==0){
            processWho(client);
            break;
        }
        else if(strncmp("name ",line,5)==0){
            const char* name = line+5;
            processRename(name,client);
            break;
        }
        else if(strncmp("tell ",line,5)==0){
            const char* args = line+5;
            processTell(args,client);
            break;
        }
        else if(strncmp("yell ",line,5)==0){
            const char* msg = line+5;
            processYell(msg,client);
            break;
        }
        
        error = true;
    
    }while(false);
    if(error){
        const char* str = "[Server] ERROR: Error command.\r\n";
        write(client.fd,str,strlen(str));    
    }

    return 0;
}
int processWho(FD_ARRAY_T& client){
    char buf[MAX_BUF_SIZE];
    for(FD_ARRAY_T& other : fd_array){
        if(other.fd <0) continue;
        bzero(buf,MAX_BUF_SIZE);
        if(&client==&other){ // sender
            sprintf(buf,"[Server] %s %s/%hu <-me\r\n",other.name.c_str(),other.ip,other.port);
        }   
        else{
            sprintf(buf,"[Server] %s %s/%hu\r\n",other.name.c_str(),other.ip,other.port);
        }
        write(client.fd,buf,strlen(buf));
    }
    return 0;
}
int processRename(const char* name,FD_ARRAY_T& client){
    int len = strlen(name);
    // check name validity
    printf("[%s] wants to change name to [%s]\n",client.name.c_str(),name);
    bool valid = false;
    char err_msg[MAX_BUF_SIZE];
    bzero(err_msg,MAX_BUF_SIZE);
    do{
        // check 2~12 english characters
        bool char_err = true;
        do{
            if(len<2 || len > 12){
                break;
            }
            bool chk_err = false;
            for(int i=0;i<len&&!chk_err;i++){
                if(!isalpha(name[i])){
                    chk_err = true;
                }
            }
            if(chk_err){
                break;
            }


            char_err = false;
        }while(false);
        if(char_err){ 
            sprintf(err_msg,"ERROR: Username can only consists of 2~12 English letters.\r\n");
            break;
        }
        
        // check anonymous
        if(strcmp(name,"anonymous")==0){
            sprintf(err_msg,"[Server] ERROR: Username cannot be anonymous.\r\n");
            break;
        }
        // check unique
        bool chk_err = false;
        for(FD_ARRAY_T& other:fd_array){
            if(other.fd < 0 || &other == &client) continue;
            if(strcmp(name,other.name.c_str())==0){
                chk_err = true;
                sprintf(err_msg,"[Server] ERROR: %s has been used by others.\r\n",name);
                break;
            }
        }
        if(chk_err) break;


        valid = true;
        
    }while(false);
    if(!valid){
        write(client.fd,err_msg,strlen(err_msg));    
    }   
    else{
        // change name
        char to_me[MAX_BUF_SIZE];
        char to_other[MAX_BUF_SIZE];
        sprintf(to_me,"[Server] You're now known as %s.\r\n",name);
        sprintf(to_other,"[Server] %s is now known as %s.\r\n",client.name.c_str(),name);
        int me_sz = strlen(to_me);
        int other_sz = strlen(to_other);
        client.name = name;
        const char* msg;
        int msg_sz;
        for(FD_ARRAY_T& other:fd_array){
            if(other.fd < 0) continue;
            if(&other == &client){
                msg = to_me;
                msg_sz = me_sz;
            }
            else{
                msg = to_other;
                msg_sz = other_sz;
            }
            write(other.fd,msg,msg_sz);
        }

    }
    return 0;   
}




int processTell(const char* args,FD_ARRAY_T& client){
    if(client.name == "anonymous"){
        const char* str = "[Server] ERROR: You are anonymous.\r\n";
        write(client.fd,str,strlen(str));        
    }
    else{
        // prepare data
        stringstream ss;
        ss << string(args);
        string target_name;
        ss >> target_name;
        string msg;
        ss >> msg;
        // check target anonymous
        if(target_name=="anonymous"){    
            const char* str = "[Server] ERROR: The client to which you sent is anonymous.\r\n";
            write(client.fd,str,strlen(str));        
        }
        else{
            // search for target object
            FD_ARRAY_T* pTarget = NULL;
            for(auto& other : fd_array){
                if(other.fd >=0 && target_name==other.name){
                    pTarget = &other;
                    break;
                }
            }
            if(!pTarget){ // target not exist
                const char* str = "[Server] ERROR: The receiver doesn't exist.\r\n";
                write(client.fd,str,strlen(str));        
            }
            else{
                auto& target = *pTarget;
                // to sender
                const char* str = "[Server] SUCCESS: Your message has been sent.\r\n";
                write(client.fd,str,strlen(str)); 
                // to target
                char buf[MAX_BUF_SIZE];
                snprintf(buf,MAX_BUF_SIZE-3,"[Server] %s tell you %s\r\n",client.name.c_str(),msg.c_str());
                buf[MAX_BUF_SIZE-3] = '\r';
                buf[MAX_BUF_SIZE-2] = '\n';
                buf[MAX_BUF_SIZE-1] = '\0';
                
                write(target.fd,buf,strlen(buf));
            }
        }
    }   
    return 0;
}
int processYell(const char* msg,FD_ARRAY_T& client){
    char buf[MAX_BUF_SIZE];
    snprintf(buf,MAX_BUF_SIZE-3,"[Server] %s yell %s\r\n",client.name.c_str(),msg);
    buf[MAX_BUF_SIZE-3] = '\r';
    buf[MAX_BUF_SIZE-2] = '\n';
    buf[MAX_BUF_SIZE-1] = '\0';
    int len = strlen(buf);
    for(auto& target : fd_array){
        if(target.fd < 0) continue;
        write(target.fd,buf,len);
    }   
    return 0;
}

*/
