#include "netdb.h"
#include <iostream>
#include "cstdio"
#include "arpa/inet.h"

using namespace std;

int main(){
    

    string name;
    char buf[INET_ADDRSTRLEN];
    while(cout << "Enter hostname to search:",cin >> name){
        auto pHost = gethostbyname(name.c_str());
        if(pHost){
            printf("Search result for %s : \n",name.c_str());
            printf("Alias : \n");
            char** alias;
            for(alias = pHost->h_aliases;*alias != NULL;alias++){
                printf("-->%s\n",*alias); 
            }
            printf("IP : \n");
            char** addr;
            for(addr = pHost->h_addr_list;*addr != NULL;addr++){
                printf("-->%s\n",inet_ntop(pHost->h_addrtype,*addr,buf,sizeof(buf)));
            }

        }
        else{
            printf("Error occur when searching for %s\n",name.c_str());
        }

    }
}
