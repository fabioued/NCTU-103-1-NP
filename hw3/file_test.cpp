#include "iostream"
#include "non-block-file.h"

using namespace std;

int main(){
    fileInfo file;
    file.load("server.cpp");
    cout << "File Size:" << file.size << endl;
}

