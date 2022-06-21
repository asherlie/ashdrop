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

/* stores queues of received requests by mac address */
// TODO: this is not needed anymore with new separate kq popping implementation


/*
this must be blocking so that threads can wait on mac addr messages
once messages arrive, we can broadcast the relevant chunk
gonna be harder than i thought tho - bc all connecotrs are getting this...
damn each connector who wants a file might need a separate mtype
and an actual kq reader in each spawned thread

this is the new design -

offer up file
thread pops control mtype kq waiting for requests
connectors/requesters say they want the file

control assigns port for this comm and communicates it to requester
first person to respond on that channel

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

#if !1
final implementation

[we offer a file] - optional

control_thread receives a file request containing name and size - (both must match) optional
the requester knows about this info either from a file offer or user input

control thread assigns a port to the tranasfer and communicates it with mac address to the receiver
control thread spawns a new thread to read from specified port - all messages being received in this thread will be ignored if they
are not from the expected mac address

#endif

struct file{
    char* fn;
    pthread_mutex_t transfer_lock;
    struct file* next;
};

struct filesys{
    struct file** buckets;
    int n_buckets;
};

struct kq_info{
    key_t key_incoming, key_outgoing;
    // to mark which ports are in use - for now there's no concurrent comm
    // but there's a limited amount of possible ports so we'll re-use old ones when needed
    _Bool in_use[0xff+1];
    uint8_t transfer_port;
    uint8_t control_port;
};

void init_fs(struct filesys* fs, int n_buckets){
    fs->n_buckets = n_buckets;
    fs->buckets = calloc(sizeof(struct file*), fs->n_buckets);
}

struct file* add_file(struct filesys* fs, char* fn){
    struct file* f = malloc(sizeof(struct file));
    int idx;
    for(char* i = fn; *i; ++i)
        idx += *i;
    f->fn = fn;
    f->next = fs->buckets[idx];
    fs->buckets[idx] = f;
    return f; 
}

void init_kq_info(struct kq_info* kqi, key_t incoming, key_t outgoing, uint8_t control_port){
    kqi->key_incoming = incoming;
    kqi->key_outgoing = outgoing;
    kqi->control_port = control_port;
    memset(kqi->in_use, 0, sizeof(uint8_t)*(0xff+1));
    for(int i = 0; i < control_port+1; ++i)
        kqi->in_use[i] = 1;
    kqi->transfer_port = control_port+1;
}

void* control_thread(void* v_kq_info){
    struct kq_info* kqi = v_kq_info;
    /*
     * reads from kq with conductor mtype/port,
     *     spawns new thread to read from specified port 
    */
    pop_kq(kqi->key_incoming, kqi->control_port, NULL);
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
// TODO: a selection system is needed - maybe get_file_offers()
// TODO: all messages should be of a standard type - int, n_data_bytes, etc.
// and accept_offer()
_Bool recv_file(struct kq_info* kqi, uint8_t comm_port){
    uint8_t* recvd = pop_kq(kqi->key_incoming, kqi->control_port, NULL);
    char* fn = (char*)recvd + sizeof(long);
    long bytes;

    memcpy(&bytes, recvd, sizeof(long));
    printf("recvd offer for file %s of size %li\n", fn, bytes);
    insert_kq(&comm_port, sizeof(uint8_t), kqi->key_outgoing, kqi->control_port);
    return 1;
}

void p_addr(uint8_t* addr){
    for(int i = 0; i < 5; ++i){
        printf("%02x:", i);
    }
    printf("%02x\n", addr[5]);
}

int main(){
}
