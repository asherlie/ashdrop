#if 0
/*

send out file offer - includes size and name

machine can accept by sending any message in the same mtype channel
otherwise, just ignore

once we're sending to someone, we send 
we can likely rely on sanity checks - no need to check bytes for now
we'll just send a 1 for a recvd packet and a 0 otherwise

this is just a one and done program
no need for any long running process

actually fuck, we could have this be a daemon and accept requests as well

for now though let's keep it simple




offer_file() broadcasts file info - name, size

interested users respond with the same mtype

a thread is always running that reads from the kq - each new user that wants the file
responds with the same mtype and a new thread is spawned to read from them
only one thread will still be reading the kq though - the kq thread just sends each thread its relevant mac address' messages

we will only send bytes when requested - the initial response to a request would be a request
for chunk i of the file

the sender thread can exit once 

the program shuts down once n users have completed their transfer

*/
#endif

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "kq.h"

/* this should ideally come from libashnet/packet_storage.h */
#define DATA_BYTES 10

struct transfer{
    int offset;
};

struct transfer_packet{
    int bytes;
};

struct fragment_request{
    int offset, bytes;
    struct fragment_request* next;
};

struct request_q{
    /*we can be clever about this because we know the number of fragments although dupes are possible*/
    struct fragment_request* first, * last;
};

struct addr_ll{
    uint8_t addr[6];
    struct request_q* rq;
    struct addr_ll* next;
};

/* stores queues of received requests by mac address */
// TODO: this is not needed anymore with new separate kq popping implementation
struct request_storage{
    struct addr_ll* buckets[1531];
};

void init_request_storage(struct request_storage* rs){
    memset(rs->buckets, 0, sizeof(struct addr_ll*)*1531);
}

void init_request_q(struct request_q* rq){
    rq->first = rq->last = NULL;
}

void insert_request_q(struct request_q* rq, int offset, int bytes){
    struct fragment_request* fr = malloc(sizeof(struct fragment_request));
    fr->offset = offset;
    fr->bytes = bytes;
    fr->next = NULL;
    if(!rq->first)rq->first = rq->last = fr;
    else{
        rq->last->next = fr;
        rq->last = fr;
    }
}

struct fragment_request* pop_request_q(struct request_q* rq){
    struct fragment_request* fr = rq->first;
    return fr;
    /*
    this must be blocking so that threads can wait on mac addr messages
    once messages arrive, we can broadcast the relevant chunk
    gonna be harder than i thought tho - bc all connecotrs are getting this...
    damn each connector who wants a file might need a separate mtype
    and an actual kq reader in each spawned thread

    FUCK this is the new design - offer up file
    thread pops control mtype kq waiting for requests
    connectors/requesters specify their mtype

    if mtype is already taken, IGNORE or send a bad mtype response
    thread is spawned to pop kqs from mtype

    this thread handles fragment_requests as they arrive - offset and nbytes
    file is read incrementally as these requests arrive
    rewinds will never be necessary because we'll only progress once we've verified prev chunk
    only one request will be fulfilled at once because each thread will be reading from the same file

    we can just have a file lock in the peer port thread

    we can get rid of the fragment_request struct but keep the mac addr storage
    mac addr storage can be used as a whitelist for receivers - nvm actually
    this can all be intercepted anyway so the file should be encrypted by sender
    if they want any kind of security
    so we can get rid of all structs
    */
}

int sum_addr(uint8_t* addr){
    int ret = 0;
    for(int i = 0; i < 6; ++i)
        ret += addr[i];
    return ret;
}

struct addr_ll* lookup_addr(struct request_storage* rs, uint8_t* addr, _Bool create){
    int idx = sum_addr(addr);
    struct addr_ll* ret;
    for(ret = rs->buckets[idx]; ret; ret = ret->next){
        if(!memcmp(ret->addr, addr, 6))break;
    }
    if(!create)return ret;
    if(!ret){
        ret = malloc(sizeof(struct addr_ll));
        ret->rq = malloc(sizeof(struct request_q));
        init_request_q(ret->rq);
        memcpy(ret->addr, addr, 6);
        ret->next = rs->buckets[idx];
        rs->buckets[idx] = ret;
    }
    return ret;
}

void* msg_reader_thread(void* xx){
    (void)xx;
    /*
     * reads from kq,
     *     if mac address is new -> create entry, spawn thread
     *     else -> add request to mac address specific request queue
     *   nvm
     *
     * reads from kq with conductor mtype/port,
     *     spawns new thread to read from specified port 
    */
    return NULL;
}

pthread_t spawn_thread(void *(*routine)(void *), void* arg){
    pthread_t ret;
    pthread_create(&ret, NULL, routine, arg);
    return ret;
}

_Bool offer_file(key_t kq, uint8_t mtype, char* fn, char* to){
    (void)to;
    FILE* fp = fopen(fn, "rb");
    long fbytes;
    uint8_t buf[sizeof(long)+50] = {0};
    size_t fnlen = strlen(fn);
    if(!fp)return 0;
    fseek(fp, 0, SEEK_END);
    fbytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    memcpy(buf, &fbytes, sizeof(long));
    memcpy(buf+sizeof(long), fn, fnlen);
    /*htonl*/
    /*need to convert to network order!*/
    /*htobe64*/
    insert_kq(buf, sizeof(long)+fnlen, kq, mtype);
    return 1;
}

// there can be a standard ashdrop control port
_Bool recv_file(key_t kq, uint8_t mtype, uint8_t comm_port){
    uint8_t* recvd = pop_kq(kq, mtype, NULL);
    char* fn = (char*)recvd + sizeof(long);
    long bytes;

    memcpy(&bytes, recvd, sizeof(long));
    printf("recvd offer for file %s of size %li\n", fn, bytes);
    return 1;
}

void p_addr(uint8_t* addr){
    for(int i = 0; i < 5; ++i){
        printf("%02x:", i);
    }
    printf("%02x\n", addr[5]);
}

int main(){
    uint8_t addr[6] = {0, 1, 2, 3, 4, 5};
    struct addr_ll* ll;
    struct request_storage rs;
    init_request_storage(&rs);
    ll = lookup_addr(&rs, addr, 1);
    insert_request_q(ll->rq, 0, DATA_BYTES);
    p_addr(ll->addr);
}
