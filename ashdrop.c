#if 0
we have a struct that stores files that are being sent
this memory is not freed until the file has been fully
sent - when we get a file complete notification

a thread runs that handles messages
a specific mtype is used for ashdrop
fchunks are built as recvd

if a message is marked as gap request, it will request
to fill in some gap - as in the nth chunk of a file

file size is broadcasted along with filename
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "kq.h"

#define DATA_BYTES 15

struct fchunk{
    int total_bytes, chunks;
    uint8_t** data;
};

void init_fchunk(struct fchunk* fc, int fsz){
    fc->total_bytes = fsz;
    fc->chunks = fsz/DATA_BYTES;
    fc->data = calloc(sizeof(uint8_t*), fc->chunks);
}

void create_fchunk(char* fn, int){
}

int broadcast_fchunk(struct fchunk* fc, key_t kq){
}

int main(){
}
