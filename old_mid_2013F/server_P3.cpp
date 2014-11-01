#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>
#include <sstream>
#ifndef PORT
#define PORT 50001
#endif

using namespace std;


int ProcessCMD(const char* input,char* reply,int index); 
const int max_buf = 2048;
const int MAX_FD_NUM = 15;

int accu_array[MAX_FD_NUM];

int main(){
    

    sockaddr_in addr_self,addr_peer;
    socklen_t len_cli;

    int fd_self,fd_peer;

    fd_self = socket(AF_INET,SOCK_STREAM,0);

    if(fd_self==0){
        cout << "Socket error." << endl;
        return 1;
    }


    addr_self.sin_family = AF_INET;
    addr_self.sin_addr.s_addr = INADDR_ANY;
    addr_self.sin_port = htons(PORT);


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cout << "bind error" << endl;
        return 1;
    }

    if(listen(fd_self,10)){
       cout << "listen error" << endl; 
        return 1;
    }





    fd_set rset,allset;
    int fd_array[MAX_FD_NUM];
    for(int i=0;i<MAX_FD_NUM;i++){
        fd_array[i] = -1;
    }
    int max_fd = fd_self;
    int next_index = 0;
    int nready;
    FD_ZERO(&allset);
    FD_SET(fd_self,&allset);

    while(true){
        rset = allset;
        cout << "Doing Select..." << endl;
        nready = select(max_fd+1,&rset,NULL,NULL,NULL);
        cout << "Ready Count:" << nready << endl;
        // listen for new client
        if(FD_ISSET(fd_self,&rset)){
            
            int inserted = -1;
            for(int i=0;i<next_index&&inserted==-1;i++){
                if(fd_array[i]==-1){
                    inserted = i;
                }
            }
            if(inserted==-1){
                if(next_index==MAX_FD_NUM){
                    cout << "No more fd to use." << endl;
                    continue;
                }
                else{
                    inserted = next_index++;   
                }
            }
            len_cli = sizeof(addr_peer);
            int new_fd = accept(fd_self,(sockaddr*)&addr_peer,&len_cli);
            fd_array[inserted] = new_fd;
            accu_array[inserted] = 0;
            FD_SET(new_fd,&allset);
            max_fd  = max(max_fd,new_fd);
            cout << "FD Arrive:" << new_fd << endl;
            cout << "Max FD:" << max_fd << endl;
            if(--nready==0){
                continue;
            }   
        } // end new
        char buf[max_buf];
        char reply[max_buf];
        for(int i=0;i<next_index;i++){
            int fd_cli = fd_array[i];
            printf("FD[%d]=%d\n",i,fd_cli);
            if(fd_cli < 0){
                continue;
            }
            if(FD_ISSET(fd_cli,&rset)){
                printf("Data from FD[%d]=%d\n",i,fd_cli);
                // New data
                bzero(buf,max_buf);
                int read_n = read(fd_cli,buf,max_buf-1);
                cout << "Read Length:" << read_n << endl;
                int end_fd = read_n <= 0;
                int stat = ProcessCMD(buf,reply,i);
                if(stat!=0){
                    cout << "Send Length:" << strlen(reply) << endl;
                    cout << "Send:" << reply;
                    write(fd_cli,reply,strlen(reply));
                    cout << "-----------" << endl;
                }else{
                    end_fd = true;
                }
                
                if(end_fd){
                    close(fd_cli);
                    fd_array[i] = -1;
                    FD_CLR(fd_cli,&allset);
                    
                }
                
                if(--nready==0){
                 //break;
                 }
            }
        }



    }


    close(fd_self);
}

int ProcessCMD(const char* input,char* reply,int index){
    cout << "Readed:" << input << endl;
    int& accu = accu_array[index];
    bzero(reply,max_buf);
    stringstream ss;
    ss << string(input);
    string cmd;
    ss >> cmd;
    cout << "CMD:" << cmd << endl;
    if(cmd=="EXIT"){
        return 0;
    }
    else{
        string op_str;
        ss >> op_str;
        int opnd = atoi(op_str.c_str());
        cout << "Opnd:" << opnd << endl;
        if(cmd=="SET"){
            accu = opnd;           
        }
        else if(cmd=="ADD"){
            accu += opnd;
        }
        else if(cmd=="SUB"){
            accu -= opnd;
        }
        sprintf(reply,"%d\r\n",accu);
        return 1;
    }
}

