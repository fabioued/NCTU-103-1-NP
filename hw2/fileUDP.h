#define MAX_BUF_SIZE 2048
#define MAX_DATA_SIZE  1400
typedef long seq_t;
struct HEADER{
    bool eof;
    seq_t offset;
    seq_t data_size;
};
