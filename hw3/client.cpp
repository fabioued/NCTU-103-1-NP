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
void showBar(int progress);

int fd_cmd;
const char* name;
const char* pHostname;
const char* pPort;
vector<fileMeta> localFiles;
vector<fileMeta> diffFiles;

int main(int argc,char** argv){
#ifdef WRITE_TO_LOG   
    freopen("Log","a",stderr);
#endif
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
        if(FD_ISSET(FD_STDIN,&rset)){ // stdin command
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
            else if(cmd=="/sleep"){
                string timeStr;
                cin >> timeStr;
                int time = stoi(timeStr);
                cout << "Client starts to sleep" << endl;
                for(int i=1;i<=time;i++){
                    cout << "Sleep " << i << endl;
                    sleep(1);
                }
                cout << "Client wakes up" << endl;
            }
        }
        if(FD_ISSET(fd_cmd,&rset)){
            // there is a new command
            // read header
            cmdHeader header;
            bool result = readObject(fd_cmd,header);
            if(result && header.cmdType == CMD_LIST){
                fprintf(stderr,"[SYNC] Passive sync for user : %s\n",name);
                retriveListHeader();
                processSync();
            }
            else{
                fprintf(stderr,"[WARNING] Server closed\n");
                return;
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
    // clear diff
    diffFiles.clear();
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
        fprintf(stderr,"[GET] Sending Header for file %s, %u\n",entry.name,entry.size);
        cmdHeader cH;
        cH.setName(name);
        cH.cmdType = CMD_GET;
        cH.meta = entry;
        sendObject(fd_data,cH);
        fprintf(stderr,"[GET] Prepare to download\n");
        // read files
        char* buf = new char[entry.size];
        bzero(buf,entry.size);
        
        
        char* ptr = buf;
        size_t each_size = entry.size / 19;
        size_t partSizes[20];
        for(int i=0;i<19;i++){
            partSizes[i] = each_size * (1+i);
        }
        partSizes[19] = entry.size;
        cout << "Downloading file : " << entry.name << endl;
        
        int showIndex = 0;
        size_t read_byte = 0;
        size_t remain = entry.size;
        while(read_byte < entry.size){
            size_t read_n;
            read_n = read(fd_data,ptr,remain);
            remain -= read_n;
            ptr += read_n;
            read_byte += read_n;
            for(int i=showIndex;i<20;i++){
                if(read_byte > partSizes[i]){
                    showBar(i);
                    usleep(1000*DELAY_MS);
                }
                else{
                    showIndex = i;
                    break;
                }
            }
                
        }
        for(int i=showIndex;i<20;i++){
            showBar(i);
        }
        

        // write to file
        ofstream fout(entry.name);
        fout.write(buf,entry.size);
        cout << endl << "Download " << entry.name << " complete!" << endl;
        delete[] buf;
        close(fd_data);
        // record to local
        localFiles.push_back(entry);
    }
}

void processUpload(const string& filename){
    // open file
    fileInfo file;
    file.load(filename);
    if(file.size > 1200*1024*1024){
        cerr << "File Too Large (exceeds 1200MB)" << endl;
        return;
    }
    fileMeta& meta = file.meta;
    // each diff meta
    fprintf(stderr,"[PUT]--> upload file : %s\n",meta.name);
    // New socket
    int fd_data = socketInit(pHostname,atoi(pPort));   
    // cmd header
    cmdHeader cH;
    cH.setName(name);
    cH.cmdType = CMD_PUT;
    cH.meta = file.meta;
    sendObject(fd_data,cH);
    // prepare send
    char* ptr = file.data;
    size_t each_size = meta.size / 19;
    size_t partSizes[20];
    for(int i=0;i<19;i++){
        partSizes[i] = each_size;
    }
    partSizes[19] = meta.size - 19*each_size;// remaining
    cout << "Uploading file : " << meta.name << endl;
    for(int i=0;i<20;i++){
        int writeSZ = partSizes[i];
        showBar(i);
        usleep(1000*DELAY_MS);
        if(writeSZ > 0){
            write(fd_data,ptr,writeSZ);
            ptr += writeSZ;
        }
    }
    cout << endl << "Upload " << meta.name << " complete!" << endl;
    close(fd_data);
    // record to local
    localFiles.push_back(meta);

}

void showBar(int progress){
    cout << "\rProgress : [";
    for(int i=0;i<=progress;i++){
        cout << '#';        
    }
    for(int i=progress+1;i<20;i++){
        cout << ' ';
    }
    cout << ']';
    cout.flush();
}
