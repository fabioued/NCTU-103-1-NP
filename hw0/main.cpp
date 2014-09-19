#include <iostream>
#include <cstring>
using namespace std;
void reverse(char* str, int size);
int main(int argc,char** argv){
   
   for(int i=0;i<argc;i++){
      cout << "argv[" << i << "]:" << argv[i] << endl;
      reverse(argv[i],strlen(argv[i]));
      cout << "reverse argv[" << i << "]:" << argv[i] << endl;
   }
   
  

   return 0;
}
void reverse(char* str, int size){
   int run_time = size / 2;
   for(int i=0;i<run_time;i++){
      char tmp = str[i];
      int index = size - 1 - i;
      str[i] = str[index];
      str[index] = tmp;
   }
}
