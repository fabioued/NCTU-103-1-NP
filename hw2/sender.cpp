#include <iostream>
#include "arpa/inet.h"
#include "unistd.h"
#include "cstdlib"
#include "cstring"
#include <cstdio>
#include <string>
#include "netdb.h"
#include "signal.h"
#include "errno.h"
#include "fstream"
#include "fileUDP.h"
#include "Unix_Timing.h"

using namespace std;



char buf[MAX_BUF_SIZE];
go_back_entry GBN_buf[GO_BACK_N];

extern bool check[GO_BACK_N];

int main(int argc,char** argv){
    
    // Timing
    Timing(TIMING_START); 

    // Usage
    if(argc < 4){
      cout << "Usage : sender.out [filename] [target address] [connect port]" << endl;
      exit(1);
    }
    char *v_filename = argv[1];
    char *v_hostname = argv[2];
    char *v_port = argv[3];




    // Get Host
    hostent* host = gethostbyname(v_hostname);
    if(!host){
      cout << "Cannot resolve host." << endl;
      exit(1);
    }


    // Socket Variables
    sockaddr_in addr_self,addr_peer;
    socklen_t len_cli;
    int fd_self,fd_peer;

    // Create Socket
    fd_self = socket(AF_INET,SOCK_DGRAM,0);
    if(fd_self==0){
        cout << "Socket error." << endl;
        return 1;
    }


    // Fill Socket Info
    addr_peer.sin_family = AF_INET;
    addr_peer.sin_port = htons(atoi(v_port));
    //memcpy(&addr_peer.sin_addr,host->h_addr,host->h_length);
    memcpy(&addr_peer.sin_addr,*host->h_addr_list,sizeof(in_addr));

    inet_ntop(host->h_addrtype,*host->h_addr_list,buf,MAX_BUF_SIZE);
    cout << "Send to IP:" << buf << " Port : "<< v_port << endl;
    
    // setup connection
    len_cli = sizeof(addr_peer);
    if(connect(fd_self,(sockaddr*)&addr_peer,len_cli)){
        cout << "Connect error" << endl;
        exit(1);
    }
   
    // set time
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100*1000;
    setsockopt(fd_self,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));


    // Send Info Init
    int recv_n;
    seq_t offset = 0,next_offset;
    int finished = 0;
    int data_count;
    bool send_ok = false;
    HEADER header,recv_header;
    ifstream file(v_filename,ios_base::in | ios_base::binary);
    /*
     * sending format: HEADER(sizeof-HEADER) data(MAX_DATA_SIZE)
     *
     */
    const int data_offset = sizeof(HEADER);
    seq_t block_offset = 0;
    const int frame_size = data_offset + MAX_DATA_SIZE;
    int send_size;
    char* send_buf = new char[frame_size+1];
    int block_read;
    cout << "Start Main Loop..." << endl;
    while(!finished){
        // load many blocks
        int i;
        resetCheck();
        for(i=0;i<GO_BACK_N&&!finished;i++){    
            //cout << "Load Block : " << i << endl;
            // load a block of data into send_buf
            file.read(GBN_buf[i].buf+data_offset,MAX_DATA_SIZE);
            data_count = file.gcount();
            // set eof
            finished = file.eof();
            // fill offset and data_size
            header.offset = block_offset;
            header.data_size = data_count;
            offset = offset + data_count;
            // copy header to send_buf
            header.index = i;
            GBN_buf[i].ptrH = (HEADER*)GBN_buf[i].buf;
            memcpy(GBN_buf[i].buf,&header,sizeof(header));
        }
        block_offset = offset;
        block_read = i;
        cout << "block read:" << block_read << endl;

RESEND:
        cout << "Start Re-send" << endl;
        /*cout << "Pending...";
        for(int i=0;i<block_read;i++){
            if(!check[i]){
                
                cout << i << ' ' ;
            }
        }*/
        cout << endl;
        // start_send
        send_ok = false;
        // send each block
        for(int i=0;i<block_read;i++)
        {
            if(check[i]){
                continue;
            }
            GBN_buf[i].ptrH->eof = finished;
            GBN_buf[i].ptrH->next_offset = offset;
            GBN_buf[i].ptrH->block_read = block_read;
            send_size = sizeof(HEADER) + GBN_buf[i].ptrH->data_size; 
            //cout << "sending ["<< i << "] , offset :" << GBN_buf[i].ptrH->offset << endl;
            write(fd_self,GBN_buf[i].buf,send_size);
        }
        do{

            len_cli = sizeof(addr_peer);
            recv_n = read(fd_self,buf,MAX_BUF_SIZE);
            if(recv_n < 0){
                if(errno== EWOULDBLOCK){
                    cout << "Socket timeout, re-send..." << endl;   
                    goto RESEND;
                }
                else{
                    cout << "The peer has close connection or unknown system error occured." << endl;
                    finished = true;
                    break;
                }
            }
            else{
                // extract header
                memcpy(&recv_header,buf,sizeof(recv_header));
                //cout << "Ack , index is " << recv_header.index << ",Block offset : " << recv_header.offset << endl;
                if(recv_header.offset < GBN_buf[0].ptrH->offset){
                    cout << "Offset " << recv_header.offset <<" is expired" << endl;   
                }
                else{ 
                    check[recv_header.index] = true; // mark received
                    if(isAllOK(block_read)){
                        send_ok = true;
                    }
                }

            }
            
        }while(!send_ok);
        // success , migrate next offset
    }
    cout << "Terminating" << endl;
    delete[] send_buf;
    close(fd_self);


    // Timing
    Timing(TIMING_END); 
    



}
