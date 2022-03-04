#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>

#include "kq.h"

struct msgbuf{
    long mtype;
    char mdata[KQ_MAX+1];
};

_Bool insert_kq(char* msg, key_t kq, uint8_t mtype){
    int msgid = msgget(kq, 0777);
    struct msgbuf buf;
    memset(&buf, 0, sizeof(struct msgbuf));
    buf.mtype = mtype;
    strncpy(buf.mdata, msg, KQ_MAX);

    /* TODO: don't use strnlen() here, insert_kq() should
     * require a len argument
     */
    return !msgsnd(msgid, &buf, strnlen(buf.mdata, KQ_MAX), 0);
}

uint8_t* pop_kq(key_t kq, uint8_t* mtype){
    int msgid = msgget(kq, 0777), br;
    struct msgbuf buf;
    uint8_t* ret;

    memset(&buf, 0, sizeof(struct msgbuf));
    br = msgrcv(msgid, &buf, KQ_MAX, 0, 0);
    ret = malloc(br);
    memcpy(ret, buf.mdata, br);
    ret[br] = 0;
    if(mtype)*mtype = buf.mtype;

    return ret;
}
