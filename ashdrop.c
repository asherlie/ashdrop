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

TODO: whitelist users
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>
#include <arpa/inet.h>

#include "kq.h"

#define DATA_BYTES 15

enum ashdrop {ASHDROP_MTYPE = 56,
              GAP_REQ,
              GAP_FILL,
              FILE_NOTIF};

struct fchunk{
    char fname[DATA_BYTES-4];
    int16_t total_bytes, chunks;
    uint8_t** data;
};

void init_fchunk(struct fchunk* fc, int fsz, char* fn){
    strncpy(fc->fname, fn, DATA_BYTES-4);
    fc->total_bytes = fsz;
    fc->chunks = (fsz/DATA_BYTES)+1;
    fc->data = calloc(sizeof(uint8_t*), fc->chunks);
}

int create_fchunk(struct fchunk* fc, char* fn){
    FILE* fp = fopen(fn, "rb");
    int bytes_recorded = 0;
    fseek(fp, 0, SEEK_END);
    init_fchunk(fc, ftell(fp), fn);
    fseek(fp, 0, SEEK_SET);
    for(int i = 0; i < fc->chunks; ++i){
        fc->data[i] = calloc(1, DATA_BYTES);
        bytes_recorded += fread(fc->data[i], 1, MIN(DATA_BYTES, fc->total_bytes-bytes_recorded), fp);
    }
    fclose(fp);

    return bytes_recorded;
}

int broadcast_fchunk(struct fchunk* fc, key_t kq){
    uint8_t hdr[DATA_BYTES] = {0};
    int16_t tb = htonl(fc->total_bytes);

    memcpy(hdr, &tb, sizeof(int16_t));
    memcpy(hdr+4, fc->fname, DATA_BYTES-4);
    insert_kq(hdr, kq, FILE_NOTIF);

    for(int i = 0; i < fc->chunks; ++i){
        insert_kq(fc->data[i], kq, ASHDROP_MTYPE);
    }

    return 0;
}

int main(){
    struct fchunk fc;
    printf("%i bytes\n", create_fchunk(&fc, "ashdrop.c"));
}
