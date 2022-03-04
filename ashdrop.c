#if 0
we have a struct that stores files that are being sent
this memory is not freed until the file has been fully
sent - when we get no further gap_requests
a thread runs that

file size is broadcasted
#endif
#include <stdio.h>

#define DATA_BYTES 15

struct fchunk{
    int total_bytes;
};

int broadcast_file(){
}

int main(){
}
