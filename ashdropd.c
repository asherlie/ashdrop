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
};

struct ashdrop_msg{
    struct ashdrop_hdr hdr;
    // variable size data buffer, NULL for requests
    uint8_t* buffer;
};

struct ashdrop_entry{
    struct ashdrop_hdr file_inf;
    /* sender must be noted because file_inf.file_id
     * is only unique per sender
     */
    char* from, * fn;
    uint16_t file_id;
};

void* recv_msg_th(void* arg){
    (void)arg;
    return NULL;
}

int main(){
    return 0;
}
