#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
//#define ARGS_INSPECT
using namespace std;
// prototypes
void reverse(char* str, int size);
void run(const char* filename,const char* token);
int processLine(const char* line,const char* splitter);
int processCmd(const char* cmd,char* args,const char* splitter);
void processReverse(char* args);
void processSplit(char* args,const char* splitter);

// Main
int main(int argc,char** argv){

#ifdef ARGS_INSPECT
   // inspect the input args
   for(int i=0;i<argc;i++){
      cout << "argv[" << i << "]:" << argv[i] << endl;
      reverse(argv[i],strlen(argv[i]));
      cout << "reverse argv[" << i << "]:" << argv[i] << endl;
   }
#endif
   
   if(argc < 3){
      cout << "No enough argments."  << endl;
      cout << "usage: ./main [input file] [split token]" << endl;
      return 1;
   }
   // Run main process
   run(argv[1],argv[2]);
  

   return 0;
}
// reverse an character array (C-string)
void reverse(char* str, int size){
   int run_time = size / 2;
   for(int i=0;i<run_time;i++){
      char tmp = str[i];
      int index = size - 1 - i;
      str[i] = str[index];
      str[index] = tmp;
   }
}
// main process for this homework
// this function do file reading and stdin,
// pass every line
void run(const char* filename,const char* token){
   //cout << filename << token << endl;
   ifstream fin(filename);  
   string input;
   int result;
   while(getline(fin,input)){
      //cout << "Line:" << input << endl;
      result = processLine(input.c_str(),token);
      if(result == 0){
         //cout << "Exit in file" << endl;
         return;
      }
   }
   while(true){
      getline(cin,input);
      if(input.size()<=0){
         // Empty Line
         //cout << "Empty Line." << endl;
         continue;
      }
      result = processLine(input.c_str(),token);
      if(result == 0){
         //cout << "Exit in stdin" << endl;
         return;
      }

   }   

}
// decompose every line into command and argment,
// the argment is NOT const, so it can be used in strtok
int processLine(const char* src,const char* splitter){
   //cout << "Line:" << src << endl;
   // Copy source line
   char line[1024];
   strncpy(line,src,1023);
   line[1023] = '\0';
   // Split Command
   char* cmd;
   cmd = strtok(line," ");
   char* args;
   // finding rest string start pointer
   for(int i=0;i<1024;i++){
      if(line[i]=='\0'){
         args = line + i + 1;
         break;      
      }
   }
   // Process command and argment
   return processCmd(cmd,args,splitter);

   
}
// give command and argment,
// and do process
// return value indicates whether 'exit' is found,
// return 0 means exit
// otherwise , pass args to respecting function and return 1
int processCmd(const char* cmd,char* args,const char* splitter){
   //cout << "cmd:" << cmd << ",args:" << args << endl;
   if(strncmp(cmd,"exit",4)==0){
      // exit
      return 0;
   }  
   else if(strncmp(cmd,"reverse",7)==0){
      // reverse
      processReverse(args);
   }
   else if(strncmp(cmd,"split",5)==0){
      // split
      processSplit(args,splitter);
   }
   else{
      // Unknown
      cout << "Unknown command." << endl;
   }
   return 1;



}


void processReverse(char* args){
   //cout << "Reverse:" << args << endl;
   reverse(args,strlen(args));
   cout << args << endl;
}
void processSplit(char* args,const char* splitter){
   char* token = strtok(args,splitter);
   while(token != NULL){
      cout << token <<  ' ';
      token = strtok(NULL,splitter);
   }
   cout << endl;

}
