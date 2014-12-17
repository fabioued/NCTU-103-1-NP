#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>
#include "fileUDP.h"
#include <fstream>

using namespace std;


go_back_entry GBN_buf[GO_BACK_N];
extern bool check[GO_BACK_N];
char buf[MAX_BUF_SIZE];
char ack_buf[MAX_BUF_SIZE];


int main(int argc,char** argv){
    if(argc < 2){
      cout << "Usage : receiver.out [filename] [bind port]" << endl;
      exit(1);
    }
    char* v_filename = argv[1];
    char* v_port = argv[2];

    sockaddr_in addr_self,addr_peer;
    socklen_t len_cli;

    int fd_self,fd_peer;

    fd_self = socket(AF_INET,SOCK_DGRAM,0);

    if(fd_self==0){
        cout << "Socket error." << endl;
        return 1;
    }


    cout << "Prepare to bind UDP port : " << v_port << endl;
    addr_self.sin_family = AF_INET;
    addr_self.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_self.sin_port = htons(atoi(v_port));


    if(bind(fd_self,(sockaddr*)&addr_self,sizeof(addr_self))){
        cout << "bind error" << endl;
        exit(1);
    }
    
    int recv_n;
    ofstream file(v_filename,ios_base::out | ios_base::binary);
    seq_t cur_offset = 0; // expect to seen
    cout << "Wait for data..." << endl;
    HEADER header;
    while(true){
        len_cli = sizeof(addr_peer);
        recv_n = recvfrom(fd_self,buf,MAX_BUF_SIZE-1,0,(sockaddr*)&addr_peer,&len_cli);
        if(recv_n <= 0){
            cout << "Unexpected Error." << endl;
            exit(1);
        }
        // extract header
        memcpy(&header,buf,sizeof(HEADER));
        // check offset
        if(header.offset == cur_offset){
            //cout << "Received offset : " << cur_offset << " , data size : "  << header.data_size << ", index : " << header.index << endl;
            memcpy(GBN_buf[header.index].buf,buf,recv_n);
            GBN_buf[header.index].ptrH = (HEADER*)GBN_buf[header.index].buf;
            check[header.index] = true;
            printf("Index : %ld , block read: %ld\n",header.index,header.block_read);
            if(isAllOK(header.block_read)){
                cout << "Block " << header.offset << " OK! , next is "<<  header.next_offset << endl;
                cur_offset = header.next_offset;
                resetCheck();
                // write all
                for(int i=0;i<header.block_read;i++){
                    //cout << "Writeing..." << i << endl;
                    file.write(GBN_buf[i].buf+sizeof(HEADER),GBN_buf[i].ptrH->data_size);
                }

                if(GBN_buf[0].ptrH->eof)
                {
                    cout << "Reach EOF" << endl;
                    break;
                }
            }
            // offset as expect, attach file
        }
        else{
            cout << "Duplicate offset : " << header.offset << " , expect :" << cur_offset << endl;
        }
        // send header back
        //header.offset = cur_offset;
        memcpy(ack_buf,&header,sizeof(HEADER));
        sendto(fd_self,ack_buf,sizeof(HEADER),0,(sockaddr*)&addr_peer,len_cli);
    }

    close(fd_self);


    



}
