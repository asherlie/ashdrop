#include <stdlib.h>
#include <stdint.h>

#define KQ_MAX 1000

uint8_t* pop_kq(key_t kq, uint8_t wanted_type, uint8_t* mtype);
_Bool insert_kq(uint8_t* msg, int msglen, key_t kq, uint8_t mtype);
uint8_t* get_data(uint8_t* popped);
