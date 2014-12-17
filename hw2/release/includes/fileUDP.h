#define MAX_BUF_SIZE 2048
#define MAX_DATA_SIZE  1400
#define GO_BACK_N 1000
#define TIME_MS 50
#define TIME_ALARM 1
typedef long seq_t;
struct HEADER{
    bool eof;
    seq_t offset;
    seq_t data_size;
    seq_t index;
    seq_t next_offset;
    seq_t block_read;
};
struct go_back_entry{
    char buf[MAX_BUF_SIZE];
    HEADER* ptrH;
};
bool check[GO_BACK_N];

bool isAllOK(int maxCheck = GO_BACK_N){
    for(int i=0;i<maxCheck;i++){
        if(!check[i]){
            return false;
        }
    }
    return true;
}



void resetCheck(){
    for(int i=0;i<GO_BACK_N;i++){
        check[i] = false;
    }
    
}
