
#include "iostream"
#include "string"
#include "cstring"
#include "cstdio"
#include "cstdlib"
#include <vector>

#include "non-block-file.h"

unsigned short bindPort;

using namespace std;

// function
int socketInit();
void run_server();
void processList(fdEntry& entry);
void processFileDownload(fdEntry& entry,cmdHeader header);
// global data
int fd_listen;
FDManager* pFD;
fileManager* pFM;

int main(int argc,char** argv){
    freopen("Log","a",stderr);
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
                        cout << "New FD : " << new_fd << " Arrived" << endl;
                    }
                    else{
                    
                        //fprintf(stderr,"FD %d is readable!\n",entry.fd);
                        // extract main header
                        int read_n,readSize;
                        if(entry.phase == PHASE_NONE){
                            readSize = sizeof(cmdHeader);
                        }
                        else{
                            readSize = MAX_BUF_SIZE;
                        }
                        read_n = read(entry.fd,&buf,readSize);
                        //fprintf(stderr,"Read size : %d\n",read_n);
                        if(read_n <= 0){
                            if(errno != EWOULDBLOCK){
                                cerr << "Connection Closed" << endl;
                                close(entry.fd);
                                FD_CLR(entry.fd,&allRset);
                                FD_CLR(entry.fd,&allWset);
                                entry.remove();
                            }
                        }
                        else{
                            if(entry.phase == PHASE_NONE && read_n == sizeof(cmdHeader)){
                                // set select write
                                FD_SET(entry.fd,&allWset);
                                cmdHeader& main_h = *(cmdHeader*)(buf);
                                fprintf(stdout,"Load Header from %s, command code is %d\n",main_h.name,main_h.cmdType);
                                switch(main_h.cmdType){
                                case CMD_LOGIN:
                                    fprintf(stdout,"User '%s' login success\n",main_h.name);
                                    fileM.addUser(main_h.name);
                                    strcpy(entry.name,main_h.name);
                                    entry.type = FD_CMD;
                                    processList(entry);
                                    break;
                                case CMD_GET:
                                    strcpy(entry.name,main_h.name);
                                    fprintf(stdout,"User '%s' require file download\n",entry.name);
                                    entry.phase = PHASE_DOING;
                                    entry.type = FD_GET;
                                    processFileDownload(entry,main_h);
                                    break;
                                case CMD_PUT:
                                    strcpy(entry.name,main_h.name);
                                    fprintf(stdout,"User '%s' want to upload file %s\n",entry.name,main_h.meta.name);
                                    entry.type = FD_PUT;
                                    entry.phase = PHASE_DOING;
                                    auto& user = pFM->getUserEntry(entry.name);
                                    entry.setReadFile(main_h.meta,user.filePath(main_h.meta.name));
                                    entry.meta = main_h.meta;
                                    FD_CLR(entry.fd,&allWset); // close write way
                                    FD_CLR(entry.fd,&wset); // close write way
                                    fprintf(stderr,"Close write FD for %d\n",entry.fd);
                                    break;
                                }
                                
                            }
                            else if(entry.phase == PHASE_DOING && entry.type == FD_PUT){
                                // user upload file
                                if(!entry.patchRead(buf,read_n)){
                                    // upload finished
                                    // add new file to list
                                    auto& user = fileM.getUserEntry(entry.name);
                                    user.files.push_back(entry.meta);
                                    fprintf(stdout,"User '%s' has upload file '%s'\n",user.name,entry.meta.name);
                                    // do sync
                                    // do list for all FD_CMD with same user name
                                    for(int k=0;k<FDm.size;k++){
                                        auto& other_entry = FDm.data[k];
                                        if(other_entry.used && other_entry.type == FD_CMD && strcmp(other_entry.name,entry.name)==0){
                                            // in use and is a CMD
                                            fprintf(stdout,"Push update list to %d\n",other_entry.fd);
                                            processList(other_entry);
                                        }
                                    }
                                    // close current FD
                                    FD_CLR(entry.fd,&allRset); // close read way
                                    entry.remove(); // remove

                                }
                            }
                        }

                    }
                }
                // writable?
                if(FD_ISSET(entry.fd,&wset)){ 
                    if(entry.type != FD_LISTEN){
                        if(!entry.flushWrite() && entry.type == FD_GET){ // no more can write
                            FD_CLR(entry.fd,&allWset); 
                            FD_CLR(entry.fd,&allRset); 
                            printf("FD %d out of data\n",entry.fd);
                            entry.remove(); // mark invalid
                        }
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



void processList(fdEntry& entry){
    const char* name = entry.name;
    // main header
    cmdHeader header;
    header.cmdType = CMD_LIST;
    header.setName(name);
    entry.setData(&header,sizeof(header)); 
    // get user entry
    auto& user = pFM->getUserEntry(name);
    // list header
    transferHeader tH = user.generateListHeader();
    fprintf(stderr,"Transfer Header Ready\n");
    entry.setData(&tH,sizeof(tH));
    // file meta header
    fprintf(stderr,"Sending Meta...\n");
    for(auto& FEntry : user.files){
        // send each meta
        entry.setData(&FEntry,sizeof(FEntry));
    }
    
    fprintf(stderr,"List Response Ready\n");
}

void processFileDownload(fdEntry& entry,cmdHeader header){
    // retrive transfer header
    fprintf(stdout,"[GET] user '%s' require file '%s'\n",entry.name,header.meta.name);
    // get user entry
    auto& user = pFM->getUserEntry(entry.name);
    // get that file
    fileInfo file;
    file.load(user.filePath(header.meta.name));
    // push this file to send buffer
    entry.setData(file.data,file.size);
} 
