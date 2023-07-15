#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

enum ad_op {chunk_request, fn_request, fulfillment, inspect};

struct ashdrop_hdr{
    enum ad_op operation;
    uint16_t file_id;
    uint16_t chunksz;
    uint16_t offset;
    uint16_t totalbytes;
    // variable size data buffer, NULL for requests
    uint8_t* buffer;
};

void* recv_msg_th(void* arg){
}

int main(){
    return 0;
}
