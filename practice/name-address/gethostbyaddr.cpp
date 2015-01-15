#include "netdb.h"
#include <iostream>
#include "cstdio"
#include "arpa/inet.h"

using namespace std;

int main(){
    

    string name;
    char buf[INET_ADDRSTRLEN];
    in_addr ipv4addr;
    while(cout << "Enter IP to search:",cin >> name){
        if(!inet_pton(AF_INET, name.c_str(), &ipv4addr)){
            printf("Error occur when parsing address for for %s\n",name.c_str());
            continue;
        }
        auto pHost = gethostbyaddr(&ipv4addr,sizeof(ipv4addr),PF_INET);
        if(pHost){
            printf("Search result for %s : \n",name.c_str());
            printf("Hostname : %s\n",pHost->h_name);
            printf("Alias : \n");
            char** alias;
            for(alias = pHost->h_aliases;*alias != NULL;alias++){
                printf("-->%s\n",*alias); 
            }

        }
        else{
            printf("Error occur when searching for %s\n",name.c_str());
        }

    }
}
