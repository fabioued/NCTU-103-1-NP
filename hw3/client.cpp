#include "non-block-file.h"

#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <vector>

using namespace std;

int socketInit(const char* ip,unsigned short port);
void run_client();
void clientLogin();
void updateDiffFiles();
void retriveListHeader();
void processSync();
void processUpload(const string& filename);

int fd_cmd;
const char* name;
const char* pHostname;
const char* pPort;
vector<fileMeta> localFiles;
vector<fileMeta> diffFiles;

int main(int argc,char** argv){
    
    if(argc<4){
        cerr << "Usage : ./client <ip> <port> <username>" << endl;
        exit(1);
    }
    pHostname = argv[1];
    pPort = argv[2];
    fd_cmd = socketInit(pHostname,atoi(pPort));   
    name = argv[3];
    run_client();

    close(fd_cmd);

}
int socketInit(const char* hostname,unsigned short port){
    struct hostent* host;

    host = gethostbyname(hostname);

    if(!host){
        cout << "Host name error" << endl;
        exit(1);
    }
    sockaddr_in addr_self;
    int fd_self;

    addr_self.sin_family = PF_INET;
    addr_self.sin_port = htons(port);
    memcpy(&addr_self.sin_addr,host->h_addr,host->h_length);

    fd_self = socket(AF_INET,SOCK_STREAM,0);

    if(fd_self < 0){
        cout << "create socket failed." << endl;
        exit(1);
    }

    if(connect(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))<0){
        cout << "connect failed." << endl;
        exit(1);
    }


    return fd_self;


}


void run_client(){
    clientLogin();

    // use select to read from stdin and socket
    fd_set allset,rset;
    FD_ZERO(&allset);
    FD_SET(fd_cmd,&allset);
    FD_SET(FD_STDIN,&allset);
    int maxfdp1 = max(fd_cmd,FD_STDIN) + 1;
    while(true){
        rset = allset;
        select(maxfdp1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(FD_STDIN,&rset)){
            // read command
            string cmd;
            cin >> cmd;
            if(cmd=="/exit"){
                return;
            }
            else if(cmd=="/put"){
                string filename;
                cin >> filename;
                processUpload(filename);
            }
        }
    }

}

// send login message to server
void clientLogin(){
    // send login header
    cmdHeader header;
    header.cmdType = CMD_LOGIN;
    header.setName(name);
    sendObject(fd_cmd,header);
    // retrive response
    readObject(fd_cmd,&header,sizeof(header));
    if(header.cmdType == CMD_LIST){
        printf("Welcome to the dropbox-like server! : %s\n",name);
        retriveListHeader();
        processSync();
    }
}

// extract header and
// set all non-exist in diff
void retriveListHeader(){
    transferHeader tH;
    readObject(fd_cmd,&tH,sizeof(tH));
    fprintf(stderr,"[LIST] remote file total:%u\n",tH.tCount);
    // retrive file meta
    fileMeta meta;
    for(int i=0;i<tH.tCount;i++){
        readObject(fd_cmd,&meta,sizeof(meta));
        fprintf(stderr,"[LIST]--> filename:%s, filesize:%u",meta.name,meta.size);
        // find if exist in local files
        bool find = false;
        for(auto& local : localFiles){
           if(local == meta){
               find = true;
               fprintf(stderr,"(exist in local)\n");
               break;
           }
        }
        if(!find){
            fprintf(stderr,"(local not existed)\n");
            diffFiles.push_back(meta);    
        }

    }
}

// create each socket per file
void processSync(){
    fprintf(stderr,"[DIFF] prepare to sync\n");
    for(auto& entry : diffFiles){
        // each diff meta
        fprintf(stderr,"[DIFF]--> retrive file : %s\n",entry.name);
        // New socket
        int fd_data = socketInit(pHostname,atoi(pPort));   
        // cmd header
        cmdHeader cH;
        cH.setName(name);
        cH.cmdType = CMD_GET;
        cH.meta = entry;
        sendObject(fd_data,cH);
        // read files
        char* buf = new char[entry.size];
        char* ptr = buf;
        size_t each_size = entry.size / 19;
        size_t partSizes[20];
        for(int i=0;i<19;i++){
            partSizes[i] = each_size;
        }
        partSizes[19] = entry.size - 19*each_size;// remaining
        cout << "Downloading file : " << entry.name << endl;
        cout << "Progress : [";
        for(int i=0;i<20;i++){
            int readSZ = partSizes[i];
            cout << "#";
            cout.flush();
            usleep(1000);
            if(readSZ > 0){
                read(fd_data,ptr,readSZ);
                ptr += readSZ;
            }
        }
        cout << "]" << endl;;
        // write to file
        ofstream fout(entry.name);
        fout.write(buf,entry.size);
        cout << "Download " << entry.name << " complete!" << endl;
        delete[] buf;
        close(fd_data);
    }
}

void processUpload(const string& filename){
    // send put header
    cmdHeader header;
    header.cmdType = CMD_PUT;
    header.setName(name);
    strcpy(header.meta.name,filename.c_str());
    sendObject(fd_cmd,header);
}

