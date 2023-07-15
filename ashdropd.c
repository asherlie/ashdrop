#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#define ASHDROP_MTYPE 941

enum ad_op {chunk_request, fn_request, fulfillment, inspect};

/* each message begins with this header */
struct ashdrop_hdr{
    enum ad_op operation;
    uint16_t file_id;
    uint16_t chunksz;
    uint16_t offset;
    uint16_t n_chunks;
};

struct ashdrop_msg{
    struct ashdrop_hdr hdr;
    // variable size data buffer, NULL for requests
    uint8_t* buffer;
};

/* both sender and receiver store a version of this
 * ssender does not populate buffer
 */
struct ashdrop_entry{
    struct ashdrop_hdr file_inf;
    /* sender must be noted because file_inf.file_id
     * is only unique per sender
     */
    char* from, * fn;
    /* chunks should be set atomically - allocate memory, set
     * to value, CAS() if NULL
     */
    _Atomic uint8_t** chunks;
    /* this may not be necessary - we can always initialize chunks to NULL
     * and allocate their mem as we receive chunks
     */
    _Bool* chunk_filled;
};

void init_ashdrop_entry(struct ashdrop_entry* ae, struct ashdrop_msg* m){
    memcpy(&ae->file_inf, &m->hdr, sizeof(struct ashdrop_hdr));
    ae->from = NULL;
    ae->fn = NULL;
    ae->chunks = calloc(sizeof(uint8_t*), ae->file_inf.n_chunks);
    /*
     * for(int i = 0; i < ae->file_inf.n_chunks; ++i){
     *     ae->chunks[i] = calloc(ae->file_inf.chunksz, sizeof(uint8_t));
     * }
    */
    ae->chunk_filled = calloc(sizeof(_Bool), ae->file_inf.n_chunks);
}

/* this is sender/receiver agnostic */
void evaluate_msg(){
}

// pops messages from kq, adds them to queues OR just evaluates them here using evaluate_msg()
// the from field will be populated with the first two sections of the kq string
// "%hx:%hx:%hx:%hx:%hx:%hx,%s,%s",
// messages will be popped from kq with the ASHDROP_MTYPE
void* pop_msg_th(void* arg){
    (void)arg;
    return NULL;
}

int main(){
    return 0;
}
