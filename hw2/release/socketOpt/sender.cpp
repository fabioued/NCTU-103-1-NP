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
    tv.tv_usec = 40*1000;
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
    const int frame_size = data_offset + MAX_DATA_SIZE;
    int send_size;
    char* send_buf = new char[frame_size+1];
    while(!finished){
        // load a block of data into send_buf
        file.read(send_buf+data_offset,MAX_DATA_SIZE);
        data_count = file.gcount();
        // set eof
        finished = header.eof = file.eof();
        // fill offset and data_size
        header.offset = offset;
        header.data_size = data_count;
        next_offset = offset + data_count;
        // copy header to send_buf
        memcpy(send_buf,&header,sizeof(header));
        // start_send
        send_size = sizeof(header) + data_count; 
        send_ok = false;
        do{
            cout << "sending offset :" << offset << endl;
            write(fd_self,send_buf,send_size);
            len_cli = sizeof(addr_peer);
            recv_n = read(fd_self,buf,MAX_BUF_SIZE);
            if(recv_n < 0){
                if(errno== EWOULDBLOCK){
                    cout << "Socket timeout, re-send..." << endl;   
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
                cout << "Ack , next offset is " << recv_header.offset << endl;
                if(recv_header.offset == next_offset)  
                {
                    send_ok = true;
                }
                // else, re-send
            }
            
        }while(!send_ok);
        // success , migrate next offset
        offset = next_offset;
    }
    cout << "Terminating" << endl;
    delete[] send_buf;
    close(fd_self);


    // Timing
    Timing(TIMING_END); 
    



}
