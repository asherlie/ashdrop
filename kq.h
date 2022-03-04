#include <stdlib.h>
#include <stdint.h>

#define KQ_MAX 1000

uint8_t* pop_kq(key_t kq, uint8_t* mtype);
_Bool insert_kq(char* msg, key_t kq, uint8_t mtype);
